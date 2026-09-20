#include "systems/combat/projectile_spawn.h"

#include "objects/direction.h"
#include "objects/entities/player.h"
#include "objects/weapons/projectile.h"
#include "systems/combat/damage_source.h"

namespace combat
{

std::unique_ptr<Projectile> spawnProjectile(Player& player, int fps)
{
  if (player.getAttackCooldownRemaining() > 0)
  {
    return nullptr;
  }

  // "fire" a projectile in the player's last-faced direction starting on
  // the player's own tile.
  Direction direction = player.getLastDirection();
  const Coordinate spawnPos = player.getPosition();
  const Weapon& weapon = player.getWeapon();

  auto projectile = std::make_unique<Projectile>(
      spawnPos, direction, weaponDamage(weapon), weapon.speed, weapon.range,
      weapon.ammoSymbol);

  const int cooldownFrames = weapon.fireRate > 0 ? fps / weapon.fireRate : fps;
  player.setAttackCooldownRemaining(cooldownFrames);

  return projectile;
}

void tickAttackCooldown(Player& player)
{
  int remaining = player.getAttackCooldownRemaining();
  if (remaining > 0)
  {
    player.setAttackCooldownRemaining(remaining - 1);
  }
}

}  // namespace combat
