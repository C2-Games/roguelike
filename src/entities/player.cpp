#include "entities/player.h"

#include "core/coordinate.h"
#include "entities/entity.h"
#include "entities/fov.h"

Player::Player(int x, int y, int health, int speed, int sightRx, int sightRy)
    : Entity(x, y, '@', health, speed),
      maxHealth(health),
      sightRx(sightRx),
      sightRy(sightRy),
      fov(ellipseFOV(sightRx, sightRy)) {}

void Player::takeDamage(int damage) {
  health -= damage;
  if (health < 0) health = 0;
}

void Player::setSightRadius(int rx, int ry) {
  sightRx = rx;
  sightRy = ry;
  fov = ellipseFOV(rx, ry);
}