#include "systems/combat/projectile_movement.h"

#include "objects/entities/entity.h"
#include "objects/room/room.h"
#include "objects/weapons/projectile.h"
#include "systems/combat/damage_application.h"

namespace combat
{

void advanceProjectile(Projectile& projectile, Room& room, Player& player)
{
  for (int i = 0; i < projectile.getTilesPerTick(); ++i)
  {
    Coordinate candidate = projectile.getPosition() + projectile.getDirection();

    // check before touching tile.
    if (candidate.x < 0 || candidate.x >= Room::WIDTH || candidate.y < 0 ||
        candidate.y >= Room::HEIGHT)
    {
      projectile.deactivate();
      return;
    }

    // isWalkable() stops the projectile.
    if (!room.tiles[candidate.x][candidate.y].isWalkable())
    {
      projectile.deactivate();
      return;
    }

    // stop on the first live entity (enemy or player) standing on the
    // candidate tile.
    if (Entity* target = room.entityAt(candidate, player))
    {
      projectile.moveTo(candidate);
      projectile.deactivate();
      applyDamage(*target, projectile.getDamage());
      return;
    }

    // advance and check remaining range.
    projectile.moveTo(candidate);
    projectile.consumeRange();
    if (projectile.getRemainingRange() <= 0)
    {
      projectile.deactivate();
      return;
    }
  }
}

}  // namespace combat
