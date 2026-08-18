#include "world/objects/projectile.h"

#include "world/map/room.h"
#include "world/objects/projectile_context.h"

Projectile::Projectile(Coordinate position, Coordinate direction, int damage,
                       int tilesPerTick, int range, ColorPair color)
    : position_(position),
      direction_(direction),
      damage_(damage),
      tilesPerTick_(tilesPerTick),
      remainingRange_(range),
      color_(color)
{}

void Projectile::update(const ProjectileContext& ctx)
{
  for (int i = 0; i < tilesPerTick_; ++i)
  {
    Coordinate candidate = position_ + direction_;

    // Check before touching tile.
    if (candidate.x < 0 || candidate.x >= Room::WIDTH || candidate.y < 0 ||
        candidate.y >= Room::HEIGHT)
    {
      deactivate();
      return;
    }

    // isWalkable() stops the projectile.
    if (!ctx.room.tiles[candidate.x][candidate.y].isWalkable())
    {
      deactivate();
      return;
    }

    // Try to damage an entity at the candidate tile.
    if (ctx.tryHit(candidate, damage_))
    {
      deactivate();
      return;
    }

    // Advance and check remaining range.
    position_ = candidate;
    --remainingRange_;
    if (remainingRange_ <= 0)
    {
      deactivate();
      return;
    }
  }
}
