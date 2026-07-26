#include "entities/enemy.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>

#include "core/coordinate.h"
#include "world/level.h"
#include "world/pathfinding.h"
#include "world/room.h"
#include "world/tile.h"

namespace {
// A bunch of helper functions for enemy movement and pathfinding.

// 4-connected neighbor offsets (N, E, S, W). Matches the offsets used inside
// pathfinding.cpp so enemy movement stays consistent with the goal map.
constexpr std::array<int, 4> kDx = {0, 1, 0, -1};
constexpr std::array<int, 4> kDy = {-1, 0, 1, 0};

// True when (x, y) lies inside the fixed-size room grid.
bool inBounds(int x, int y) {
  return x >= 0 && x < Room::WIDTH && y >= 0 && y < Room::HEIGHT;
}

// True when some OTHER live enemy currently sits on `coord`. Dead enemies
// don't block; the caller itself is skipped via pointer identity so an enemy
// never blocks its own step.
bool isOccupiedByOtherEnemy(
    const Enemy* self, Coordinate coord,
    const std::vector<std::unique_ptr<Enemy>>& allEnemies) {
  return std::any_of(allEnemies.begin(), allEnemies.end(),
                     [&](const std::unique_ptr<Enemy>& e) {
                       return e.get() != self && e->isAlive() &&
                              e->getPosition() == coord;
                     });
}

// Pick a strictly-decreasing goal-map neighbor to step onto.
//
// - Filters neighbors to those with dist < map[pos] (must move CLOSER; skips
//   unreachable, equal, and uphill tiles).
// - Sorts remaining candidates by distance ascending. Random tiebreak within
//   each distance class (via prior shuffle + stable_sort).
// - Walks the sorted list and returns the first tile not occupied by another
//   live enemy. This implements the "fall back to next-best neighbor on
//   collision" policy.
//
// Returns `pos` unchanged when there are no valid down-gradient moves (goal
// unreachable, enemy already on goal, or every reachable neighbor is
// occupied).
Coordinate stepDownGradient(
    const Enemy* self, Coordinate pos, const GoalMap& map,
    const std::vector<std::unique_ptr<Enemy>>& allEnemies) {
  int currentDist = map[pos.x][pos.y];
  if (currentDist == kUnreachable || currentDist == 0) return pos;

  struct Cand {
    Coordinate coord;
    int dist;
  };
  std::vector<Cand> candidates;
  candidates.reserve(4);

  for (int i = 0; i < 4; ++i) {
    int nx = pos.x + kDx[i];
    int ny = pos.y + kDy[i];
    if (!inBounds(nx, ny)) continue;
    int d = map[nx][ny];
    if (d >= currentDist) continue;  // must strictly decrease
    candidates.push_back({Coordinate(nx, ny), d});
  }

  // Fisher-Yates shuffle for random tiebreaking. std::rand() matches the
  // RNG choice elsewhere in the codebase (Level::spawnEnemiesForRoom,
  // room_library.cpp).
  for (std::size_t i = candidates.size(); i > 1; --i) {
    std::size_t j = static_cast<std::size_t>(std::rand()) % i;
    std::swap(candidates[i - 1], candidates[j]);
  }
  std::stable_sort(
      candidates.begin(), candidates.end(),
      [](const Cand& a, const Cand& b) { return a.dist < b.dist; });

  auto it =
      std::find_if(candidates.begin(), candidates.end(), [&](const Cand& c) {
        return !isOccupiedByOtherEnemy(self, c.coord, allEnemies);
      });
  return it != candidates.end() ? it->coord : pos;
}

// Pick a random walkable Floor neighbor not occupied by another enemy.
// Walls, Void, Pillars, and Doors are all excluded — matches the blocking
// rules used by computeGoalMap so wander behavior stays consistent with
// chase.
Coordinate pickWanderTile(
    const Enemy* self, Coordinate pos, const Room& room,
    const std::vector<std::unique_ptr<Enemy>>& allEnemies) {
  std::vector<Coordinate> candidates;
  candidates.reserve(4);
  for (int i = 0; i < 4; ++i) {
    int nx = pos.x + kDx[i];
    int ny = pos.y + kDy[i];
    if (!inBounds(nx, ny)) continue;
    if (room.tiles[nx][ny].getType() != TileType::Floor) continue;
    if (isOccupiedByOtherEnemy(self, Coordinate(nx, ny), allEnemies)) continue;
    candidates.push_back(Coordinate(nx, ny));
  }
  if (candidates.empty()) return pos;
  return candidates[static_cast<std::size_t>(std::rand()) % candidates.size()];
}

}  // namespace

Enemy::Enemy(int x, int y, char symbol, int health, int speed, int attackDamage,
             FOV attackFOV, int chaseMemoryDuration)
    : Entity(x, y, symbol, health, speed),
      attackDamage(attackDamage),
      attackFOV(attackFOV),
      chaseMemoryDuration(chaseMemoryDuration),
      chaseTurnsRemaining(0),
      lastKnownPlayerPos(std::nullopt) {}

void Enemy::moveTowardPlayer(
    Coordinate playerPos, Level& level,
    const std::vector<std::unique_ptr<Enemy>>& allEnemies) {
  const bool inFoV = attackFOV.in(position, playerPos);

  // Memory refresh runs every frame so the enemy locks on the moment the
  // player enters its FoV, regardless of speed throttling.
  if (inFoV) {
    lastKnownPlayerPos = playerPos;
    chaseTurnsRemaining = chaseMemoryDuration;
  }

  // Decide what tile the enemy is trying to reach this frame.
  std::optional<Coordinate> target;
  if (inFoV) {
    target = playerPos;
  } else if (chaseTurnsRemaining > 0 && lastKnownPlayerPos.has_value()) {
    if (position == *lastKnownPlayerPos) {
      // Arrived at the last-known tile but the player has since moved.
      // Give up and fall through to idle wander.
      lastKnownPlayerPos.reset();
      chaseTurnsRemaining = 0;
    } else {
      target = *lastKnownPlayerPos;
    }
  }

  // Determine the tile to attempt to step onto.
  Coordinate nextTile;
  if (target.has_value()) {
    const GoalMap& map = level.getGoalMap(level.getCurrentRoomID(), *target);
    Coordinate chosen = stepDownGradient(this, position, map, allEnemies);
    if (chosen == position) {
      // No legal down-gradient step (target unreachable or all reachable
      // neighbors blocked by other enemies). Wander instead so the enemy
      // still feels alive.
      nextTile =
          pickWanderTile(this, position, level.getCurrentRoom(), allEnemies);
    } else {
      nextTile = chosen;
    }
  } else {
    // Idle — never spotted the player, or memory just expired. Wander.
    nextTile =
        pickWanderTile(this, position, level.getCurrentRoom(), allEnemies);
  }

  // Apply the intent. Entity::moveTo throttles by speed, so this may or may
  // not update `position` this frame.
  const Coordinate oldPos = position;
  moveTo(nextTile);

  // Chase memory ticks down per ACTUAL move (not per frame). Only counts
  // while the player is out of FoV; in-FoV frames already reset the counter
  // above.
  if (!inFoV && !(position == oldPos) && chaseTurnsRemaining > 0) {
    --chaseTurnsRemaining;
    if (chaseTurnsRemaining == 0) lastKnownPlayerPos.reset();
  }
}

void Enemy::takeDamage(int damage) {
  health -= damage;
  if (health < 0) health = 0;
}
