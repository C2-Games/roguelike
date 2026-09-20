#include "systems/combat/projectile_spawn.h"

#include "objects/direction.h"
#include "objects/entities/entity.h"
#include "objects/weapons/projectile.h"
#include "systems/combat/damage_source.h"

namespace combat
{

std::unique_ptr<Projectile> spawnProjectile(Entity& entity, int fps)
{
  if (entity.getAttackCooldownRemaining() > 0)
  {
    return nullptr;
  }

  // "fire" a projectile in the entity's last-faced direction starting on
  // the entity's own tile.
  Direction direction = entity.getLastDirection();
  const Coordinate spawnPos = entity.getPosition();
  const Weapon& weapon = entity.getWeapon();

  auto projectile = std::make_unique<Projectile>(
      spawnPos, direction, weaponDamage(weapon), weapon.speed, weapon.range,
      weapon.ammoSymbol);

  const int cooldownFrames = weapon.fireRate > 0 ? fps / weapon.fireRate : fps;
  entity.setAttackCooldownRemaining(cooldownFrames);

  return projectile;
}

void tickAttackCooldown(Entity& entity)
{
  int remaining = entity.getAttackCooldownRemaining();
  if (remaining > 0)
  {
    entity.setAttackCooldownRemaining(remaining - 1);
  }
}

}  // namespace combat
