#include "systems/combat/projectile_spawn.h"

#include "objects/direction.h"
#include "objects/entities/player.h"
#include "objects/weapons/projectile.h"
#include "systems/combat/damage_source.h"

namespace combat
{

std::unique_ptr<Projectile> spawnProjectile(const Player& player)
{
  // "fire" a projectile in the player's last-faced direction starting on
  // the player's own tile.
  Direction direction = player.getLastDirection();
  const Coordinate spawnPos = player.getPosition();
  const Weapon& weapon = player.getWeapon();

  return std::make_unique<Projectile>(spawnPos, direction, weaponDamage(weapon),
                                      weapon.getSpeed(), weapon.getRange(),
                                      weapon.getColor());
}

}  // namespace combat
