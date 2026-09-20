#include "systems/combat/damage_source.h"

#include "objects/weapons/weapon.h"

namespace combat
{
Damage weaponDamage(const Weapon& weapon) { return weapon.damage; }
}  // namespace combat
