#include "systems/combat/projectile_movement.h"

#include <algorithm>

#include "objects/direction.h"
#include "objects/entities/enemy.h"
#include "objects/entities/entity.h"
#include "objects/entities/player.h"
#include "objects/room/room.h"
#include "objects/weapons/projectile.h"
#include "systems/combat/damage_application.h"

namespace combat
{

void advanceProjectile(Projectile& projectile, const Room& room,
                       const std::vector<std::unique_ptr<Enemy>>& enemies,
                       Player& player)
{
  for (int i = 0; i < projectile.getTilesPerTick(); ++i)
  {
    Coordinate candidate =
        projectile.getPosition() + toOffset(projectile.getDirection());

    // isWalkable() stops the projectile (also covers out-of-bounds, since
    // Room::isWalkable treats anything outside the grid as unwalkable).
    if (!room.isWalkable(candidate))
    {
      projectile.deactivate();
      return;
    }

    // stop on the first live entity (enemy or player) standing on the
    // candidate tile.
    if (room.isOccupied(candidate))
    {
      projectile.moveTo(candidate);
      projectile.deactivate();

      auto hit = std::find_if(
          enemies.begin(), enemies.end(),
          [&candidate](const std::unique_ptr<Enemy>& enemy) {
            return enemy->isAlive() && enemy->getPosition() == candidate;
          });
      Entity* target = hit != enemies.end() ? hit->get() : nullptr;
      if (target == nullptr && player.isAlive() &&
          player.getPosition() == candidate)
      {
        target = &player;
      }
      if (target != nullptr)
      {
        applyDamage(*target, projectile.getDamage());
      }
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
