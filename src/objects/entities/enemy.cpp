#include "objects/entities/enemy.h"

#include <utility>

#include "objects/colors.h"
#include "objects/coordinate.h"
#include "objects/damage/damage.h"
#include "objects/damage/damage_type.h"
#include "objects/weapons/weapon.h"

Enemy::Enemy(Coordinate position, std::unique_ptr<FOV> fov, EntitySymbol symbol,
             int health, int speed, int attackDamage, int chaseMemoryDuration)
    : Entity(position, std::move(symbol), health, speed, std::move(fov),
             Weapon{Damage{DamageType::Base, 8, 0.0},
                    colorForDamageType(DamageType::Base),
                    "Enemy Bolt",
                    8,
                    1,
                    1,
                    2,
                    '*',
                    '.',
                    {}}),
      attackDamage_(attackDamage),
      chaseMemoryDuration_(chaseMemoryDuration),
      chaseTurnsRemaining_(0),
      lastKnownPlayerPos_(std::nullopt),
      aiState_(AIState::Sentry)
{}
