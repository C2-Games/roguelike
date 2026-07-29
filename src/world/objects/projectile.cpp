#include "world/objects/projectile.h"

#include "world/map/room.h"
#include "world/objects/projectile_context.h"

Projectile::Projectile(Coordinate position, Coordinate direction, int damage,
                       int tilesPerTick, int range, ColorPair color)
    : position(position),
      direction(direction),
      damage(damage),
      tilesPerTick(tilesPerTick),
      remainingRange(range),
      color(color) {}

void Projectile::update(const ProjectileContext& ctx) {
  for (int i = 0; i < tilesPerTick; ++i) {
    Coordinate candidate = position + direction;

    // Check before touching tile.
    if (candidate.x < 0 || candidate.x >= Room::WIDTH || candidate.y < 0 ||
        candidate.y >= Room::HEIGHT) {
      deactivate();
      return;
    }

    // isWalkable() stops the projectile.
    if (!ctx.room.tiles[candidate.x][candidate.y].isWalkable()) {
      deactivate();
      return;
    }

    // Try to damage an entity at the candidate tile. The context's tryHit
    // closure encapsulates "who lives on this tile" so Projectile stays
    // decoupled from the enemy vector layout.
    if (ctx.tryHit(candidate, damage)) {
      deactivate();
      return;
    }

    // Advance and check remaining range.
    position = candidate;
    --remainingRange;
    if (remainingRange <= 0) {
      deactivate();
      return;
    }
  }
}
