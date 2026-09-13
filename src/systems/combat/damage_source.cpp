#include "systems/combat/damage_source.h"

#include "objects/weapons/weapon.h"

namespace combat
{
Damage weaponDamage(const Weapon& weapon) { return weapon.damage; }

Damage meleeDamage(int amount) { return Damage{DamageType::Base, amount, 0.0}; }
}  // namespace combat
