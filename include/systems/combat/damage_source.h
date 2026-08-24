#ifndef DAMAGE_SOURCE_H
#define DAMAGE_SOURCE_H

#include "objects/damage/damage.h"

struct Weapon;

namespace combat
{
// Build the Damage a projectile fired from `weapon` deals on a hit.
Damage weaponDamage(const Weapon& weapon);

// Build the Damage a melee attacker with the given base attack (e.g.
// Enemy::getAttackDamage()) deals on a hit.
Damage meleeDamage(int amount);
}  // namespace combat

#endif
