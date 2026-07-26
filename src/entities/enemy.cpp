#include "entities/enemy.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "core/coordinate.h"

Enemy::Enemy(int x, int y, char symbol, int health, int speed, int attackDamage,
             FOV attackFOV)
    : Entity(x, y, symbol, health, speed),
      attackDamage(attackDamage),
      attackFOV(attackFOV) {}

void Enemy::moveTowardPlayer(Coordinate playerPos) {
  // only move toward player if in FOV.
  if (attackFOV.in(position, playerPos)) {
    bool needsX = position.x != playerPos.x;
    bool needsY = position.y != playerPos.y;

    int newX = position.x + (position.x < playerPos.x ? 1 : -1);
    int newY = position.y + (position.y < playerPos.y ? 1 : -1);

    Coordinate newPos = position;
    if (needsX && needsY) {
      // only randomly choose when both movements can be made.
      if (std::rand() % 2 == 0) {
        newPos.y = newY;
      } else {
        newPos.x = newX;
      }
    } else if (needsX) {
      newPos.x = newX;
    } else if (needsY) {
      newPos.y = newY;
    }

    moveTo(newPos);
  }
}

void Enemy::takeDamage(int damage) {
  health -= damage;
  if (health < 0) health = 0;
}
