#ifndef DAMAGE_SOURCE_H
#define DAMAGE_SOURCE_H

#include "objects/damage/damage.h"

struct Weapon;

namespace combat
{
// Build the Damage a projectile fired from `weapon` deals on a hit.
Damage weaponDamage(const Weapon& weapon);
}  // namespace combat

#endif
