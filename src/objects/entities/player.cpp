#include "objects/entities/player.h"

#include "objects/colors.h"
#include "objects/coordinate.h"
#include "objects/damage/damage.h"
#include "objects/damage/damage_type.h"
#include "objects/entities/entity.h"
#include "objects/fovs/ellipse_fov.h"

Player::Player(Coordinate position, int health, int speed)
    : Entity(position, EntitySymbol{{L'@'}}, health, speed,
             std::make_unique<EllipseFOV>(18, 9)),
      maxHealth_(health),
      weapon_{Damage{DamageType::Base, 10, 0.0},
              colorForDamageType(DamageType::Base),
              "Basic Bolt",
              15,
              2,
              1,
              2,
              '*',
              '.'}
{}

void Player::changeFOV(int rx, int ry)
{
  fov_ = std::make_unique<EllipseFOV>(rx, ry);
}