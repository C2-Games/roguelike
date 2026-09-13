#include "preload/weapon_catalog.h"

#include <SQLiteCpp/SQLiteCpp.h>

#include <map>
#include <string>
#include <utility>

#include "objects/colors.h"
#include "objects/damage/damage.h"
#include "objects/damage/damage_type.h"

namespace
{

// maps the damage_type column text to its enum value, defaulting to base.
DamageType parseDamageType(const std::string& text)
{
  if (text == "poison")
  {
    return DamageType::Poison;
  }
  if (text == "electric")
  {
    return DamageType::Electric;
  }
  if (text == "fire")
  {
    return DamageType::Fire;
  }
  return DamageType::Base;
}

}  // namespace

WeaponCatalog::WeaponCatalog(SQLite::Database& database)
{
  SQLite::Statement statement(
      database,
      "SELECT weapons.name, weapons.symbol, weapons.ammo_symbol, "
      "weapon_tiers.tier, weapon_tiers.damage_amount, "
      "weapon_tiers.damage_type, weapon_tiers.damage_duration, "
      "weapon_tiers.speed, weapon_tiers.range, weapon_tiers.fire_rate FROM "
      "weapons JOIN weapon_tiers ON weapon_tiers.weapon_id = weapons.id");

  while (statement.executeStep())
  {
    const std::string name = statement.getColumn(0).getString();
    auto catalogEntry = catalog_.find(name);
    if (catalogEntry == catalog_.end())
    {
      catalogEntry = catalog_.emplace(name, std::map<int, Weapon>{}).first;
    }

    const int tier = statement.getColumn(3).getInt();
    const DamageType damageType =
        parseDamageType(statement.getColumn(5).getString());

    catalogEntry->second[tier] = Weapon{
        Damage{damageType, statement.getColumn(4).getInt(),
               statement.getColumn(6).getDouble()},
        colorForDamageType(damageType),
        catalogEntry->first.c_str(),
        statement.getColumn(8).getInt(),
        statement.getColumn(9).getInt(),
        tier,
        statement.getColumn(7).getInt(),
        statement.getColumn(1).getString()[0],
        statement.getColumn(2).getString()[0],
    };
  }
}

const Weapon* WeaponCatalog::find(const std::string& name, int tier) const
{
  auto namedEntry = catalog_.find(name);
  if (namedEntry == catalog_.end())
  {
    return nullptr;
  }

  const std::map<int, Weapon>& tiers = namedEntry->second;
  auto tierEntry = tiers.find(tier);
  if (tierEntry == tiers.end())
  {
    return nullptr;
  }

  return &tierEntry->second;
}
