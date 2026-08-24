#include "objects/entities/enemy.h"

#include <algorithm>
#include <utility>

#include "objects/coordinate.h"

namespace
{
// Frames between attack attempts
constexpr int ATTACK_COOLDOWN_FRAMES = 30;
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

std::optional<Coordinate> Enemy::planMove(bool inFoV, Coordinate playerPos)
{
  transitionState(inFoV, playerPos);

  switch (aiState_)
  {
    case AIState::Chase:
      return playerPos;
    case AIState::Search:
      return lastKnownPlayerPos_;
    case AIState::Sentry:
      return std::nullopt;
  }
  return std::nullopt;
}

bool Enemy::resolveMove(Coordinate nextTile, bool inFoV, bool wouldAttackPlayer)
{
  const Coordinate oldPos = position_;
  moveTo(nextTile);

  // Chase memory ticks down per ACTUAL move (not per frame). Only counts
  // while the player is out of FoV; in-FoV frames already reset the counter
  // in transitionState.
  if (!inFoV && !(position_ == oldPos) && chaseTurnsRemaining_ > 0)
  {
    --chaseTurnsRemaining_;
    if (chaseTurnsRemaining_ == 0)
    {
      lastKnownPlayerPos_.reset();
    }
  }

  // Disengaging resets the cooldown so the next approach starts fresh.
  if (!wouldAttackPlayer)
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

void Enemy::takeDamage(Damage damage)
{
  health_ -= damage.amount;
  health_ = std::max(health_, 0);
  setActionState(EntityActionState::Damaged);
}
