#include "systems/movement/move_enemy.h"

#include <algorithm>
#include <optional>
#include <random>
#include <utility>
#include <vector>

#include "game/services.h"
#include "objects/direction.h"
#include "objects/entities/enemy.h"
#include "objects/entities/entity_symbol.h"
#include "objects/entities/player.h"
#include "objects/room/room.h"
#include "objects/tiles/tile_type.h"
#include "systems/movement/footprint_goal_map_cache.h"
#include "systems/movement/footprint_pathfinding.h"
#include "systems/movement/move_entity.h"
#include "systems/movement/pathfinding.h"

namespace
{

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
    if (isWithinRangeOfFootprint(
            enemy.getPosition(),
            orientedSymbol(enemy.getSymbol(), enemy.getOrientation()),
            playerPos, attackRange))
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
  movement::moveEntity(enemy, nextTile);
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

// advances a single-cell enemy for one frame: refreshes its chase state,
// paths toward its current target (or wanders), and resolves any resulting
// melee attack.
bool advanceSingleCellEnemy(Enemy& enemy, const Player& player, Room& room,
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

// a footprint candidate is free when every tile it covers is either
// unoccupied or part of the mover's own current footprint (about to be
// vacated), so a translate overlapping the mover's own tiles isn't blocked
// by itself.
bool footprintSlotFree(const Room& room,
                       const std::vector<Coordinate>& candidateFootprint,
                       const std::vector<Coordinate>& currentFootprint)
{
  return std::all_of(candidateFootprint.begin(), candidateFootprint.end(),
                     [&](Coordinate tile) {
                       return !room.isOccupied(tile) ||
                              std::find(currentFootprint.begin(),
                                        currentFootprint.end(),
                                        tile) != currentFootprint.end();
                     });
}

// mirrors stepDownGradient over the (anchor, orientation) configuration
// space a multi-tile footprint moves through: up to four translate
// candidates (same orientation, strictly decreasing distance) plus one
// rotate-in-place candidate (same anchor, other orientation, strictly
// decreasing distance).
std::pair<Coordinate, Orientation> footprintStepDownGradient(
    const Enemy& enemy, const FootprintGoalMap& map, const Room& room,
    GameServices& services)
{
  const EntitySymbol& baseSymbol = enemy.getSymbol();
  const Orientation orientation = enemy.getOrientation();
  const EntitySymbol oriented = orientedSymbol(baseSymbol, orientation);
  const Coordinate origin = enemy.getPosition();
  const Coordinate anchor = anchorFromOrigin(origin, oriented);

  const GoalMap& currentMap =
      orientation == Orientation::Horizontal ? map.horizontal : map.vertical;
  const int currentDist = currentMap[anchor.x][anchor.y];
  if (currentDist == UNREACHABLE || currentDist == 0)
  {
    return {origin, orientation};
  }

  struct Cand
  {
    Coordinate anchor;
    Orientation orientation;
    int dist = 0;
  };
  std::vector<Cand> candidates;
  candidates.reserve(5);

  for (Direction direction : ALL_DIRECTIONS)
  {
    Coordinate nextAnchor = anchor + toOffset(direction);
    if (!Room::inBounds(nextAnchor))
    {
      continue;
    }
    int dist = currentMap[nextAnchor.x][nextAnchor.y];
    if (dist == UNREACHABLE || dist >= currentDist)
    {
      continue;  // must strictly decrease.
    }
    candidates.push_back({nextAnchor, orientation, dist});
  }

  const Orientation rotated = orientation == Orientation::Horizontal
                                  ? Orientation::Vertical
                                  : Orientation::Horizontal;
  const GoalMap& rotatedMap =
      rotated == Orientation::Horizontal ? map.horizontal : map.vertical;
  const int rotatedDist = rotatedMap[anchor.x][anchor.y];
  if (rotatedDist != UNREACHABLE && rotatedDist < currentDist)
  {
    candidates.push_back({anchor, rotated, rotatedDist});
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

  // occupancy is only actually mutated later, in resolveFootprintMove.
  const std::vector<Coordinate> currentFootprint =
      footprintTiles(origin, oriented);
  auto freeCandidate = std::find_if(
      candidates.begin(), candidates.end(), [&](const Cand& candidate) {
        const EntitySymbol candOriented =
            orientedSymbol(baseSymbol, candidate.orientation);
        const Coordinate candOrigin =
            originFromAnchor(candidate.anchor, candOriented);
        const std::vector<Coordinate> candFootprint =
            footprintTiles(candOrigin, candOriented);
        return footprintSlotFree(room, candFootprint, currentFootprint);
      });

  if (freeCandidate == candidates.end())
  {
    return {origin, orientation};
  }
  const EntitySymbol freeOriented =
      orientedSymbol(baseSymbol, freeCandidate->orientation);
  return {originFromAnchor(freeCandidate->anchor, freeOriented),
          freeCandidate->orientation};
}

// mirrors pickWanderTile over the same (anchor, orientation) candidate
// shape as footprintStepDownGradient, filtering by blocking tiles and
// self-overlap instead of a goal-map gradient.
std::pair<Coordinate, Orientation> footprintPickWanderMove(
    const Enemy& enemy, const Room& room, GameServices& services)
{
  const EntitySymbol& baseSymbol = enemy.getSymbol();
  const Orientation orientation = enemy.getOrientation();
  const EntitySymbol oriented = orientedSymbol(baseSymbol, orientation);
  const Coordinate origin = enemy.getPosition();
  const Coordinate anchor = anchorFromOrigin(origin, oriented);
  const std::vector<Coordinate> currentFootprint =
      footprintTiles(origin, oriented);

  auto footprintIsFree = [&](Coordinate candOrigin,
                             const EntitySymbol& candOriented) {
    const std::vector<Coordinate> candFootprint =
        footprintTiles(candOrigin, candOriented);
    return std::none_of(
               candFootprint.begin(), candFootprint.end(),
               [&](Coordinate tile) { return isBlocking(room, tile); }) &&
           footprintSlotFree(room, candFootprint, currentFootprint);
  };

  struct Cand
  {
    Coordinate anchor;
    Orientation orientation;
  };
  std::vector<Cand> candidates;
  candidates.reserve(5);

  for (Direction direction : ALL_DIRECTIONS)
  {
    Coordinate nextAnchor = anchor + toOffset(direction);
    if (!Room::inBounds(nextAnchor))
    {
      continue;
    }
    const Coordinate nextOrigin = originFromAnchor(nextAnchor, oriented);
    if (footprintIsFree(nextOrigin, oriented))
    {
      candidates.push_back({nextAnchor, orientation});
    }
  }

  const Orientation rotated = orientation == Orientation::Horizontal
                                  ? Orientation::Vertical
                                  : Orientation::Horizontal;
  const EntitySymbol rotatedOriented = orientedSymbol(baseSymbol, rotated);
  const Coordinate rotatedOrigin = originFromAnchor(anchor, rotatedOriented);
  if (footprintIsFree(rotatedOrigin, rotatedOriented))
  {
    candidates.push_back({anchor, rotated});
  }

  if (candidates.empty())
  {
    return {origin, orientation};
  }
  std::uniform_int_distribution<std::size_t> pick(0, candidates.size() - 1);
  const Cand& chosen = candidates[pick(services.movementRng)];
  const EntitySymbol chosenOriented =
      orientedSymbol(baseSymbol, chosen.orientation);
  return {originFromAnchor(chosen.anchor, chosenOriented), chosen.orientation};
}

// moves the enemy to a new (origin, orientation) state, updating occupancy
// over its full footprint and chase memory.
void resolveFootprintMove(Enemy& enemy, Room& room, Coordinate nextOrigin,
                          Orientation nextOrientation, bool inFoV)
{
  const Coordinate oldOrigin = enemy.getPosition();
  const Orientation oldOrientation = enemy.getOrientation();

  for (Coordinate tile : footprintTiles(
           oldOrigin, orientedSymbol(enemy.getSymbol(), oldOrientation)))
  {
    room.toggleOccupied(tile, false);
  }

  movement::moveEntity(enemy, nextOrigin, nextOrientation);

  for (Coordinate tile : footprintTiles(
           enemy.getPosition(),
           orientedSymbol(enemy.getSymbol(), enemy.getOrientation())))
  {
    room.toggleOccupied(tile, true);
  }

  if (!inFoV &&
      (enemy.getPosition() != oldOrigin ||
       enemy.getOrientation() != oldOrientation) &&
      enemy.getChaseTurnsRemaining() > 0)
  {
    enemy.setChaseTurnsRemaining(enemy.getChaseTurnsRemaining() - 1);
    if (enemy.getChaseTurnsRemaining() == 0)
    {
      enemy.setLastKnownPlayerPos(std::nullopt);
    }
  }
}

// footprint-aware mirror of advanceSingleCellEnemy, routed through
// configuration-space (anchor, orientation) candidates instead of single
// tiles.
bool advanceFootprintEnemy(Enemy& enemy, const Player& player, Room& room,
                           const FootprintGoalMapCache& cache,
                           GameServices& services)
{
  const Coordinate playerPos = player.getPosition();
  const bool inFoV = enemy.inFOV(playerPos);
  std::optional<Coordinate> target = planMove(enemy, inFoV, playerPos);

  if (enemy.getAIState() == AIState::Attack)
  {
    return true;
  }

  std::pair<Coordinate, Orientation> next;
  if (target.has_value())
  {
    const FootprintGoalMap& map =
        cache.getOrCompute(room, *target, enemy.getSymbol());
    next = footprintStepDownGradient(enemy, map, room, services);
    if (next.first == enemy.getPosition() &&
        next.second == enemy.getOrientation())
    {
      // no legal down-gradient step (target unreachable or all reachable
      // neighbors blocked by other enemies). wander instead so the enemy
      // still feels alive.
      next = footprintPickWanderMove(enemy, room, services);
    }
  }
  else
  {
    // sentry — never spotted the player, or memory just expired. wander.
    next = footprintPickWanderMove(enemy, room, services);
  }

  resolveFootprintMove(enemy, room, next.first, next.second, inFoV);
  return false;
}

}  // namespace

namespace movement
{

bool advanceEnemy(Enemy& enemy, const Player& player, Room& room,
                  const GoalMapCache& cache,
                  const FootprintGoalMapCache& footprintCache,
                  GameServices& services)
{
  if (isSingleCell(enemy.getSymbol()))
  {
    return advanceSingleCellEnemy(enemy, player, room, cache, services);
  }
  return advanceFootprintEnemy(enemy, player, room, footprintCache, services);
}

}  // namespace movement
