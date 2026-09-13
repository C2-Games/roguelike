#ifndef ENEMY_CATALOG_H
#define ENEMY_CATALOG_H

#include <map>
#include <memory>
#include <string>

#include "objects/entities/entity_symbol.h"
#include "objects/fovs/fov.h"

namespace SQLite
{
class Database;
}

// resolved per-tier stats for one named enemy.
struct EnemyTierAttributes
{
  EntitySymbol symbol;
  int health = 0;
  int attackDamage = 0;
  std::unique_ptr<FOV> fov;
  int chaseMemoryDuration = 0;
  int speed = 0;
};

// loads every enemy row from the database into an in-memory lookup of
// name -> tier -> resolved attributes.
class EnemyCatalog
{
 public:
  /**
   * @brief Query every enemy/tier row from an open database.
   *
   * @param database Open database connection to read enemies/enemy_tiers
   * from.
   */
  explicit EnemyCatalog(SQLite::Database& database);

  /**
   * @brief Look up a name/tier combination.
   *
   * @param name Enemy name as authored in its config (e.g. "goblin").
   * @param tier Tier number (e.g. 1 for "tier_1").
   * @return Pointer to the resolved attributes, or nullptr if not found.
   */
  const EnemyTierAttributes* find(const std::string& name, int tier) const;

 private:
  std::map<std::string, std::map<int, EnemyTierAttributes>> catalog_;
};

#endif
