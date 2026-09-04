#ifndef DB_SEED_WEAPONS_H
#define DB_SEED_WEAPONS_H

#include <string>

namespace SQLite
{
class Database;
}

namespace db
{

/**
 * @brief Read every weapon json under `<assetsDir>/weapons/` and insert the
 * resulting rows into the weapons/weapon_tiers tables.
 *
 * @param database Open database connection to insert into.
 * @param assetsDir Path to the repo's assets directory (the parent of
 * `weapons/`).
 */
void seedWeapons(SQLite::Database& database, const std::string& assetsDir);

}  // namespace db

#endif
