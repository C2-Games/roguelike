#include "entities/player.h"

#include "core/coordinate.h"
#include "entities/entity.h"
#include "entities/fov.h"

Player::Player(int x, int y, int health, int speed, int sightRx, int sightRy)
    : Entity(x, y, '@', health, speed),
      maxHealth_(health),
      sightRx_(sightRx),
      sightRy_(sightRy),
      fov_(ellipseFOV(sightRx, sightRy)) {}

void Player::takeDamage(int damage) {
  health_ -= damage;
  if (health_ < 0) health_ = 0;
}

void Player::setSightRadius(int rx, int ry) {
  sightRx_ = rx;
  sightRy_ = ry;
  fov_ = ellipseFOV(rx, ry);
}