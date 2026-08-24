#include "objects/entities/enemy.h"

#include <utility>

#include "objects/coordinate.h"

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
