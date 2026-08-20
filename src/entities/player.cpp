#include "entities/player.h"

#include "core/coordinate.h"
#include "entities/entity.h"
#include "entities/fov.h"

Player::Player(Coordinate position, int health, int speed)
    : Entity(position, EntitySymbol{{'@'}}, health, speed, ellipseFOV(16, 10)),
      maxHealth_(health)
{}

void Player::takeDamage(int damage)
{
  health_ -= damage;
  if (health_ < 0) health_ = 0;
}

void Player::changeFOV(int rx, int ry) { fov_ = ellipseFOV(rx, ry); }