#include "entities/enemy.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <random>
#include <utility>

#include "core/frame_state.h"
#include "core/services.h"
#include "entities/player.h"
#include "objects/coordinate.h"
#include "objects/tiles/tile.h"
#include "objects/tiles/tile_type.h"
#include "world/map/room.h"
#include "world/systems/goal_map_cache.h"
#include "world/systems/pathfinding.h"

namespace
{
// A bunch of helper functions for enemy movement and pathfinding.

// 4-connected neighbor offsets (N, E, S, W). Matches the offsets used inside
// pathfinding.cpp so enemy movement stays consistent with the goal map.
constexpr std::array<int, 4> kDx = {0, 1, 0, -1};
constexpr std::array<int, 4> kDy = {-1, 0, 1, 0};

// Frames between attack attempts
constexpr int ATTACK_COOLDOWN_FRAMES = 30;

// True when (x, y) lies inside the fixed-size room grid.
bool inBounds(int x, int y)
{
  return x >= 0 && x < Room::WIDTH && y >= 0 && y < Room::HEIGHT;
}

// Outcome of a single down-gradient step attempt.
struct GradientStep
{
  Coordinate nextTile;     // pos unchanged if no move (blocked or attacking).
  bool wouldAttackPlayer;  // true when the best down-gradient neighbor is the
                           // player's tile.
};

// Pick a strictly-decreasing goal-map neighbor to step onto.
//
// Returns `pos` unchanged (with `wouldAttackPlayer = false`) when there are
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
      continue;  // must strictly decrease
    }
    candidates.push_back({Coordinate(nx, ny), d});
  }

  // Fisher-Yates shuffle for random tiebreaking. Uses the injected RNG so
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

Enemy::Enemy(Coordinate position, std::unique_ptr<FOV> fov, EntitySymbol symbol,
             int health, int speed, int attackDamage, int chaseMemoryDuration)
    : Entity(position, std::move(symbol), health, speed, std::move(fov)),
      attackDamage_(attackDamage),
      chaseMemoryDuration_(chaseMemoryDuration),
      chaseTurnsRemaining_(0),
      lastKnownPlayerPos_(std::nullopt),
      aiState_(AIState::Sentry),
      attackCooldownRemaining_(0)
{}

void Enemy::transitionState(bool inFoV, Coordinate playerPos)
{
  // Memory refresh runs every frame so the enemy locks on the moment the
  // player enters its FoV, regardless of speed throttling.
  if (inFoV)
  {
    lastKnownPlayerPos_ = playerPos;
    chaseTurnsRemaining_ = chaseMemoryDuration_;
    setAIState(AIState::Chase);
    return;
  }

  // Cascading (not else-if): a single call can fall Chase -> Search ->
  // Sentry in one frame, matching a fresh re-evaluation every frame.
  if (aiState_ == AIState::Chase)
  {
    setAIState(AIState::Search);
  }

  if (aiState_ == AIState::Search)
  {
    if (chaseTurnsRemaining_ <= 0 || !lastKnownPlayerPos_.has_value())
    {
      setAIState(AIState::Sentry);
    }
    else if (position_ == *lastKnownPlayerPos_)
    {
      // Arrived at the last-known tile but the player has since moved.
      lastKnownPlayerPos_.reset();
      chaseTurnsRemaining_ = 0;
      setAIState(AIState::Sentry);
    }
  }
}

void Enemy::setAIState(AIState next)
{
  if (next == aiState_)
  {
    return;
  }
  aiState_ = next;
  switch (aiState_)
  {
    case AIState::Sentry:
      onEnterSentry();
      break;
    case AIState::Chase:
      onEnterChase();
      break;
    case AIState::Search:
      onEnterSearch();
      break;
  }
}

void Enemy::onEnterSentry() {}

void Enemy::onEnterChase() {}

void Enemy::onEnterSearch() {}

bool Enemy::moveTowardPlayer(const FrameState& frame, const GoalMapCache& cache,
                             GameServices& services)
{
  const Coordinate playerPos = frame.player.getPosition();
  const bool inFoV = fov_->in(position_, playerPos);

  transitionState(inFoV, playerPos);

  // Decide what tile the enemy is trying to reach this frame.
  std::optional<Coordinate> target;
  switch (aiState_)
  {
    case AIState::Chase:
      target = playerPos;
      break;
    case AIState::Search:
      target = lastKnownPlayerPos_;
      break;
    case AIState::Sentry:
      break;
  }

  // Determine the tile to attempt to step onto. wasBlockedByPlayer tracks
  // whether the best down-gradient step this frame is the player's tile.
  Coordinate nextTile;
  bool wasBlockedByPlayer = false;
  if (target.has_value())
  {
    const GoalMap& map = cache.getOrCompute(frame.currentRoom, *target);
    GradientStep step = stepDownGradient(
        this, position_, map, frame.currentRoom, playerPos, services);
    if (step.wouldAttackPlayer)
    {
      wasBlockedByPlayer = true;
      nextTile = position_;
    }
    else if (step.nextTile == position_)
    {
      // No legal down-gradient step (target unreachable or all reachable
      // neighbors blocked by other enemies). Wander instead so the enemy
      // still feels alive.
      nextTile = pickWanderTile(this, position_, frame.currentRoom, services);
    }
    else
    {
      nextTile = step.nextTile;
    }
  }
  else
  {
    // Sentry — never spotted the player, or memory just expired. Wander.
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
    if (chaseTurnsRemaining_ == 0)
    {
      lastKnownPlayerPos_.reset();
    }
  }

  // Disengaging resets the cooldown so the next approach starts fresh.
  if (!wasBlockedByPlayer)
  {
    attackCooldownRemaining_ = 0;
    return false;
  }
  if (attackCooldownRemaining_ > 0)
  {
    --attackCooldownRemaining_;
    return false;
  }

  // Minus one: this frame is itself part of the gap.
  attackCooldownRemaining_ = ATTACK_COOLDOWN_FRAMES - 1;
  setActionState(EntityActionState::Attack);
  return true;
}

void Enemy::takeDamage(int damage)
{
  health_ -= damage;
  health_ = std::max(health_, 0);
  setActionState(EntityActionState::Damaged);
}
