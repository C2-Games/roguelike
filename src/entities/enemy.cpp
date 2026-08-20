#include "entities/enemy.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <random>
#include <utility>

#include "core/coordinate.h"
#include "core/frame_state.h"
#include "core/services.h"
#include "entities/player.h"
#include "world/map/room.h"
#include "world/map/tile.h"
#include "world/systems/goal_map_cache.h"
#include "world/systems/pathfinding.h"

namespace
{
// A bunch of helper functions for enemy movement and pathfinding.

// 4-connected neighbor offsets (N, E, S, W). Matches the offsets used inside
// pathfinding.cpp so enemy movement stays consistent with the goal map.
constexpr std::array<int, 4> kDx = {0, 1, 0, -1};
constexpr std::array<int, 4> kDy = {-1, 0, 1, 0};

// True when (x, y) lies inside the fixed-size room grid.
bool inBounds(int x, int y)
{
  return x >= 0 && x < Room::WIDTH && y >= 0 && y < Room::HEIGHT;
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
Coordinate stepDownGradient(const Enemy* self, Coordinate pos,
                            const GoalMap& map, const Room& room,
                            GameServices& services)
{
  int currentDist = map[pos.x][pos.y];
  if (currentDist == kUnreachable || currentDist == 0) return pos;

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
    if (!inBounds(nx, ny)) continue;
    int d = map[nx][ny];
    if (d >= currentDist) continue;  // must strictly decrease
    candidates.push_back({Coordinate(nx, ny), d});
  }

  // Fisher-Yates shuffle for random tiebreaking. Uses the injected RNG so
  // enemy behavior is reproducible when the game seed is pinned.
  for (std::size_t i = candidates.size(); i > 1; --i)
  {
    std::uniform_int_distribution<std::size_t> pick(0, i - 1);
    std::size_t j = pick(services.rng);
    std::swap(candidates[i - 1], candidates[j]);
  }
  std::stable_sort(
      candidates.begin(), candidates.end(),
      [](const Cand& a, const Cand& b) { return a.dist < b.dist; });

  auto freeCandidate = std::find_if(
      candidates.begin(), candidates.end(),
      [&](const Cand& c) { return room.enemyAt(c.coord, self) == nullptr; });
  return freeCandidate != candidates.end() ? freeCandidate->coord : pos;
}

// Pick a random walkable Floor neighbor not occupied by another enemy.
// Walls, Void, Pillars, and Doors are all excluded — matches the blocking
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
    if (!inBounds(nx, ny)) continue;
    if (room.tiles[nx][ny].getType() != TileType::Floor) continue;
    if (room.enemyAt(Coordinate(nx, ny), self) != nullptr) continue;
    candidates.push_back(Coordinate(nx, ny));
  }
  if (candidates.empty()) return pos;
  std::uniform_int_distribution<std::size_t> pick(0, candidates.size() - 1);
  return candidates[pick(services.rng)];
}

}  // namespace

Enemy::Enemy(Coordinate position, std::unique_ptr<FOV> fov, EntitySymbol symbol,
             int health, int speed, int attackDamage, int chaseMemoryDuration)
    : Entity(position, std::move(symbol), health, speed, std::move(fov)),
      attackDamage_(attackDamage),
      chaseMemoryDuration_(chaseMemoryDuration),
      chaseTurnsRemaining_(0),
      lastKnownPlayerPos_(std::nullopt)
{}

void Enemy::moveTowardPlayer(const FrameState& frame, const GoalMapCache& cache,
                             GameServices& services)
{
  const Coordinate playerPos = frame.player.getPosition();
  const bool inFoV = fov_->in(position_, playerPos);

  // Memory refresh runs every frame so the enemy locks on the moment the
  // player enters its FoV, regardless of speed throttling.
  if (inFoV)
  {
    lastKnownPlayerPos_ = playerPos;
    chaseTurnsRemaining_ = chaseMemoryDuration_;
  }

  // Decide what tile the enemy is trying to reach this frame.
  std::optional<Coordinate> target;
  if (inFoV)
  {
    target = playerPos;
  }
  else if (chaseTurnsRemaining_ > 0 && lastKnownPlayerPos_.has_value())
  {
    if (position_ == *lastKnownPlayerPos_)
    {
      // Arrived at the last-known tile but the player has since moved.
      // Give up and fall through to idle wander.
      lastKnownPlayerPos_.reset();
      chaseTurnsRemaining_ = 0;
    }
    else
    {
      target = *lastKnownPlayerPos_;
    }
  }

  // Determine the tile to attempt to step onto.
  Coordinate nextTile;
  if (target.has_value())
  {
    const GoalMap& map = cache.getOrCompute(frame.currentRoom, *target);
    Coordinate chosen =
        stepDownGradient(this, position_, map, frame.currentRoom, services);
    if (chosen == position_)
    {
      // No legal down-gradient step (target unreachable or all reachable
      // neighbors blocked by other enemies). Wander instead so the enemy
      // still feels alive.
      nextTile = pickWanderTile(this, position_, frame.currentRoom, services);
    }
    else
    {
      nextTile = chosen;
    }
  }
  else
  {
    // Idle — never spotted the player, or memory just expired. Wander.
    nextTile = pickWanderTile(this, position_, frame.currentRoom, services);
  }

  // Apply the intent. Entity::moveTo throttles by speed, so this may or may
  // not update `position` this frame.
  const Coordinate oldPos = position_;
  moveTo(nextTile);

  // Chase memory ticks down per ACTUAL move (not per frame). Only counts
  // while the player is out of FoV; in-FoV frames already reset the counter
  // above.
  if (!inFoV && !(position_ == oldPos) && chaseTurnsRemaining_ > 0)
  {
    --chaseTurnsRemaining_;
    if (chaseTurnsRemaining_ == 0) lastKnownPlayerPos_.reset();
  }
}

void Enemy::takeDamage(int damage)
{
  health_ -= damage;
  if (health_ < 0) health_ = 0;
}
