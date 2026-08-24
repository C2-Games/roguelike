#include "systems/movement/movement.h"

#include <algorithm>
#include <array>
#include <optional>
#include <random>
#include <utility>
#include <vector>

#include "core/services.h"
#include "objects/entities/enemy.h"
#include "objects/entities/player.h"
#include "objects/room/room.h"
#include "objects/tiles/tile.h"
#include "objects/tiles/tile_type.h"
#include "systems/movement/pathfinding.h"

namespace
{

// 4-connected neighbor offsets (N, E, S, W). matches the offsets used inside
// pathfinding.cpp so enemy movement stays consistent with the goal map.
constexpr std::array<int, 4> kDx = {0, 1, 0, -1};
constexpr std::array<int, 4> kDy = {-1, 0, 1, 0};

// true when (x, y) lies inside the fixed-size room grid.
bool inBounds(int x, int y)
{
  return x >= 0 && x < Room::WIDTH && y >= 0 && y < Room::HEIGHT;
}

// outcome of a single down-gradient step attempt.
struct GradientStep
{
  Coordinate nextTile;     // pos unchanged if no move (blocked or attacking).
  bool wouldAttackPlayer;  // true when the best down-gradient neighbor is the
                           // player's tile.
};

// pick a strictly-decreasing goal-map neighbor to step onto.
//
// returns `pos` unchanged (with `wouldAttackPlayer = false`) when there are
// no valid down-gradient moves (goal unreachable, enemy already on goal, or
// every reachable neighbor is occupied).
GradientStep stepDownGradient(const Enemy* self, Coordinate pos,
                              const GoalMap& map, const Room& room,
                              Coordinate playerPos, GameServices& services)
{
  int currentDist = map[pos.x][pos.y];
  if (currentDist == kUnreachable || currentDist == 0)
  {
    return {pos, false};
  }

  struct Cand
  {
    Coordinate coord;
    int dist;
  };
  std::vector<Cand> candidates;
  candidates.reserve(4);

  for (int i = 0; i < 4; ++i)
  {
    int nx = pos.x + kDx[i];
    int ny = pos.y + kDy[i];
    if (!inBounds(nx, ny))
    {
      continue;
    }
    int d = map[nx][ny];
    if (d >= currentDist)
    {
      continue;  // must strictly decrease.
    }
    candidates.push_back({Coordinate(nx, ny), d});
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

  if (!candidates.empty() && candidates.front().coord == playerPos)
  {
    return {pos, true};
  }

  auto freeCandidate =
      std::find_if(candidates.begin(), candidates.end(), [&](const Cand& c) {
        return c.coord != playerPos && room.enemyAt(c.coord, self) == nullptr;
      });
  return {freeCandidate != candidates.end() ? freeCandidate->coord : pos,
          false};
}

// pick a random walkable Floor neighbor not occupied by another enemy.
// walls, Void, Pillars, and Doors are all excluded — matches the blocking
// rules used by computeGoalMap so wander behavior stays consistent with
// chase.
Coordinate pickWanderTile(const Enemy* self, Coordinate pos, const Room& room,
                          GameServices& services)
{
  std::vector<Coordinate> candidates;
  candidates.reserve(4);
  for (int i = 0; i < 4; ++i)
  {
    int nx = pos.x + kDx[i];
    int ny = pos.y + kDy[i];
    if (!inBounds(nx, ny))
    {
      continue;
    }
    if (room.tiles[nx][ny].getType() != TileType::Floor)
    {
      continue;
    }
    if (room.enemyAt(Coordinate(nx, ny), self) != nullptr)
    {
      continue;
    }
    candidates.push_back(Coordinate(nx, ny));
  }
  if (candidates.empty())
  {
    return pos;
  }
  std::uniform_int_distribution<std::size_t> pick(0, candidates.size() - 1);
  return candidates[pick(services.movementRng)];
}

}  // namespace

namespace movement
{

bool advanceEnemy(Enemy& enemy, const Player& player, const Room& room,
                  const GoalMapCache& cache, GameServices& services)
{
  const Coordinate playerPos = player.getPosition();
  const bool inFoV = enemy.canSeePlayer(playerPos);
  std::optional<Coordinate> target = enemy.planMove(inFoV, playerPos);

  Coordinate nextTile;
  bool wasBlockedByPlayer = false;
  if (target.has_value())
  {
    const GoalMap& map = cache.getOrCompute(room, *target);
    GradientStep step = stepDownGradient(&enemy, enemy.getPosition(), map, room,
                                         playerPos, services);
    if (step.wouldAttackPlayer)
    {
      wasBlockedByPlayer = true;
      nextTile = enemy.getPosition();
    }
    else if (step.nextTile == enemy.getPosition())
    {
      // no legal down-gradient step (target unreachable or all reachable
      // neighbors blocked by other enemies). wander instead so the enemy
      // still feels alive.
      nextTile = pickWanderTile(&enemy, enemy.getPosition(), room, services);
    }
    else
    {
      nextTile = step.nextTile;
    }
  }
  else
  {
    // sentry — never spotted the player, or memory just expired. wander.
    nextTile = pickWanderTile(&enemy, enemy.getPosition(), room, services);
  }

  return enemy.resolveMove(nextTile, inFoV, wasBlockedByPlayer);
}

PlayerStepOutcome stepPlayer(Player& player, const Room& room,
                             Coordinate direction)
{
  const Coordinate nextPos = player.getPosition() + direction;

  if (nextPos.x < 0 || nextPos.x >= Room::WIDTH || nextPos.y < 0 ||
      nextPos.y >= Room::HEIGHT ||
      !room.tiles[nextPos.x][nextPos.y].isWalkable() ||
      room.enemyAt(nextPos) != nullptr)
  {
    return {PlayerStepKind::Blocked, Coordinate()};
  }

  if (room.tiles[nextPos.x][nextPos.y].getType() == TileType::Door)
  {
    return {PlayerStepKind::AtDoor, nextPos};
  }

  player.moveTo(nextPos);
  return {PlayerStepKind::Moved, Coordinate()};
}

}  // namespace movement
