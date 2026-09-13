#ifndef WEAPON_CATALOG_H
#define WEAPON_CATALOG_H

#include <map>
#include <string>

#include "objects/weapons/weapon.h"

namespace SQLite
{
class Database;
}

// loads every weapon row from the database into an in-memory lookup of
// name -> tier -> resolved Weapon.
class WeaponCatalog
{
 public:
  /**
   * @brief Query every weapon/tier row from an open database.
   *
   * @param database Open database connection to read weapons/weapon_tiers
   * from.
   */
  explicit WeaponCatalog(SQLite::Database& database);

  /**
   * @brief Look up a name/tier combination.
   *
   * @param name Weapon name as authored in its config (e.g. "bow").
   * @param tier Tier number (e.g. 1 for "tier_1").
   * @return Pointer to the resolved weapon, or nullptr if not found. The
   * returned Weapon's name points into memory owned by this catalog, so it
   * is only valid for as long as this WeaponCatalog instance is alive.
   */
  const Weapon* find(const std::string& name, int tier) const;

 private:
  std::map<std::string, std::map<int, Weapon>> catalog_;
};

#endif
