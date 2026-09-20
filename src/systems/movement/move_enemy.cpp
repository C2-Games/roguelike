#include "systems/movement/move_enemy.h"

#include <algorithm>
#include <cstdlib>
#include <optional>
#include <random>
#include <utility>
#include <vector>

#include "game/services.h"
#include "objects/direction.h"
#include "objects/entities/enemy.h"
#include "objects/entities/player.h"
#include "objects/room/room.h"
#include "objects/tiles/tile_type.h"
#include "systems/movement/pathfinding.h"

namespace
{

// chebyshev (king-move) distance between two coordinates.
int chebyshevDistance(Coordinate a, Coordinate b)
{
  return std::max(std::abs(a.x - b.x), std::abs(a.y - b.y));
}

// pick the next tile to step onto along a strictly-decreasing goal-map
// gradient.
//
// returns `pos` unchanged when there are no valid down-gradient moves (goal
// unreachable, enemy already on goal, or every reachable neighbor is
// occupied).
Coordinate stepDownGradient(Coordinate pos, const GoalMap& map,
                            const Room& room, GameServices& services)
{
  int currentDist = map[pos.x][pos.y];
  if (currentDist == UNREACHABLE || currentDist == 0)
  {
    return pos;
  }

  struct Cand
  {
    Coordinate coord;
    int dist = 0;
  };
  std::vector<Cand> candidates;
  candidates.reserve(4);

  for (Direction direction : ALL_DIRECTIONS)
  {
    Coordinate neighbor = pos + toOffset(direction);
    if (!room.inBounds(neighbor))
    {
      continue;
    }
    int dist = map[neighbor.x][neighbor.y];
    if (dist >= currentDist)
    {
      continue;  // must strictly decrease.
    }
    candidates.push_back({neighbor, dist});
  }

  // fisher-yates shuffle for random tiebreaking. uses the injected RNG so
  // enemy behavior is reproducible when the game seed is pinned.
  for (std::size_t i = candidates.size(); i > 1; --i)
  {
    std::uniform_int_distribution<std::size_t> pick(0, i - 1);
    std::size_t j = pick(services.movementRng);
    std::swap(candidates[i - 1], candidates[j]);
  }
  std::stable_sort(
      candidates.begin(), candidates.end(),
      [](const Cand& a, const Cand& b) { return a.dist < b.dist; });

  auto freeCandidate =
      std::find_if(candidates.begin(), candidates.end(),
                   [&](const Cand& c) { return !room.isOccupied(c.coord); });
  return freeCandidate != candidates.end() ? freeCandidate->coord : pos;
}

// pick a random walkable Floor neighbor not occupied by another enemy.
// walls, Void, Pillars, and Doors are all excluded — matches the blocking
// rules used by computeGoalMap so wander behavior stays consistent with
// chase.
Coordinate pickWanderTile(Coordinate pos, const Room& room,
                          GameServices& services)
{
  std::vector<Coordinate> candidates;
  candidates.reserve(4);
  for (Direction direction : ALL_DIRECTIONS)
  {
    Coordinate neighbor = pos + toOffset(direction);
    if (room.getTileType(neighbor) != TileType::Floor)
    {
      continue;
    }
    if (room.isOccupied(neighbor))
    {
      continue;
    }
    candidates.push_back(neighbor);
  }
  if (candidates.empty())
  {
    return pos;
  }
  std::uniform_int_distribution<std::size_t> pick(0, candidates.size() - 1);
  return candidates[pick(services.movementRng)];
}

// refreshes the enemy's AI state (sentry/chase/search) based on whether the
// player is currently visible.
void transitionAIState(Enemy& enemy, bool inFoV, Coordinate playerPos)
{
  if (inFoV)
  {
    enemy.setLastKnownPlayerPos(playerPos);
    enemy.setChaseTurnsRemaining(enemy.getChaseMemoryDuration());
    const int attackRange =
        std::min(enemy.getFOV().maxRadius(), enemy.getWeapon().range);
    if (chebyshevDistance(enemy.getPosition(), playerPos) <= attackRange)
    {
      enemy.setLastDirection(directionTowards(enemy.getPosition(), playerPos));
      enemy.setAIState(AIState::Attack);
    }
    else
    {
      enemy.setAIState(AIState::Chase);
    }
    return;
  }
  if (enemy.getAIState() == AIState::Attack ||
      enemy.getAIState() == AIState::Chase)
  {
    enemy.setAIState(AIState::Search);
  }
  if (enemy.getAIState() == AIState::Search)
  {
    if (enemy.getChaseTurnsRemaining() <= 0 ||
        !enemy.getLastKnownPlayerPos().has_value())
    {
      enemy.setAIState(AIState::Sentry);
    }
    else if (enemy.getPosition() == *enemy.getLastKnownPlayerPos())
    {
      enemy.setLastKnownPlayerPos(std::nullopt);
      enemy.setChaseTurnsRemaining(0);
      enemy.setAIState(AIState::Sentry);
    }
  }
}

// decides the enemy's current movement target, if any, from its AI state.
std::optional<Coordinate> planMove(Enemy& enemy, bool inFoV,
                                   Coordinate playerPos)
{
  transitionAIState(enemy, inFoV, playerPos);
  switch (enemy.getAIState())
  {
    case AIState::Chase:
      return playerPos;
    case AIState::Search:
      return enemy.getLastKnownPlayerPos();
    case AIState::Attack:
    case AIState::Sentry:
      return std::nullopt;
  }
  return std::nullopt;
}

// moves the enemy to `nextTile`, updating occupancy and chase memory.
void resolveMove(Enemy& enemy, Room& room, Coordinate nextTile, bool inFoV)
{
  const Coordinate oldPos = enemy.getPosition();
  room.toggleOccupied(oldPos, false);
  enemy.moveTo(nextTile);
  room.toggleOccupied(enemy.getPosition(), true);

  if (!inFoV && !(enemy.getPosition() == oldPos) &&
      enemy.getChaseTurnsRemaining() > 0)
  {
    enemy.setChaseTurnsRemaining(enemy.getChaseTurnsRemaining() - 1);
    if (enemy.getChaseTurnsRemaining() == 0)
    {
      enemy.setLastKnownPlayerPos(std::nullopt);
    }
  }
}

}  // namespace

namespace movement
{

bool advanceEnemy(Enemy& enemy, const Player& player, Room& room,
                  const GoalMapCache& cache, GameServices& services)
{
  const Coordinate playerPos = player.getPosition();
  const bool inFoV = enemy.inFOV(playerPos);
  std::optional<Coordinate> target = planMove(enemy, inFoV, playerPos);

  if (enemy.getAIState() == AIState::Attack)
  {
    return true;
  }

  Coordinate nextTile;
  if (target.has_value())
  {
    const GoalMap& map = cache.getOrCompute(room, *target);
    Coordinate step =
        stepDownGradient(enemy.getPosition(), map, room, services);
    if (step == enemy.getPosition())
    {
      // no legal down-gradient step (target unreachable or all reachable
      // neighbors blocked by other enemies). wander instead so the enemy
      // still feels alive.
      nextTile = pickWanderTile(enemy.getPosition(), room, services);
    }
    else
    {
      nextTile = step;
    }
  }
  else
  {
    // sentry — never spotted the player, or memory just expired. wander.
    nextTile = pickWanderTile(enemy.getPosition(), room, services);
  }

  resolveMove(enemy, room, nextTile, inFoV);
  return false;
}

}  // namespace movement
