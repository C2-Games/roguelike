#include "world/projectile.h"

Projectile::Projectile(Coordinate position, Coordinate direction, int damage,
                       int tilesPerTick, int range, ColorPair color)
    : position(position),
      direction(direction),
      damage(damage),
      tilesPerTick(tilesPerTick),
      remainingRange(range),
      color(color) {}

void Projectile::update(const Room& room,
                        std::vector<std::unique_ptr<Enemy>>& enemies) {
  for (int i = 0; i < tilesPerTick; ++i) {
    Coordinate candidate = position + direction;

    // check before touching tile.
    if (candidate.x < 0 || candidate.x >= Room::WIDTH || candidate.y < 0 ||
        candidate.y >= Room::HEIGHT) {
      deactivate();
      return;
    }

    // isWalkable() stops the projectile.
    if (!room.tiles[candidate.x][candidate.y].isWalkable()) {
      deactivate();
      return;
    }

    // stop on first alive enemy hit.
    bool hitEnemy = false;
    for (auto& enemy : enemies) {
      if (enemy->isAlive() && enemy->getPosition() == candidate) {
        enemy->takeDamage(damage);
        hitEnemy = true;
        break;
      }
    }
    if (hitEnemy) {
      deactivate();
      return;
    }

    // end if out of range.
    position = candidate;
    --remainingRange;
    if (remainingRange <= 0) {
      deactivate();
      return;
    }
  }
}
