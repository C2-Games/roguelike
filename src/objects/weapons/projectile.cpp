#include "objects/weapons/projectile.h"

#include "core/frame_state.h"
#include "objects/room/room.h"

Projectile::Projectile(Coordinate position, Coordinate direction, Damage damage,
                       int tilesPerTick, int range, ColorPair color)
    : position_(position),
      direction_(direction),
      damage_(damage),
      tilesPerTick_(tilesPerTick),
      remainingRange_(range),
      color_(color)
{}

bool Projectile::update(const FrameState& frame)
{
  for (int i = 0; i < tilesPerTick_; ++i)
  {
    Coordinate candidate = position_ + direction_;

    // check before touching tile.
    if (candidate.x < 0 || candidate.x >= Room::WIDTH || candidate.y < 0 ||
        candidate.y >= Room::HEIGHT)
    {
      deactivate();
      return false;
    }

    // isWalkable() stops the projectile.
    if (!frame.currentRoom.tiles[candidate.x][candidate.y].isWalkable())
    {
      deactivate();
      return false;
    }

    // stop on the first live entity (enemy or player) standing on the
    // candidate tile; actual damage is applied by the caller.
    if (frame.currentRoom.entityAt(candidate, frame.player) != nullptr)
    {
      position_ = candidate;
      deactivate();
      return true;
    }

    // advance and check remaining range.
    position_ = candidate;
    --remainingRange_;
    if (remainingRange_ <= 0)
    {
      deactivate();
      return false;
    }
  }
  return false;
}
