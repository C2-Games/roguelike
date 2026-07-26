#include "world/projectile.h"

#include <algorithm>

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
    auto hit = std::find_if(enemies.begin(), enemies.end(),
                            [&candidate](const std::unique_ptr<Enemy>& enemy) {
                              return enemy->isAlive() &&
                                     enemy->getPosition() == candidate;
                            });
    if (hit != enemies.end()) {
      (*hit)->takeDamage(damage);
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
