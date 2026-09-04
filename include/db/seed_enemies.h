#ifndef DB_SEED_ENEMIES_H
#define DB_SEED_ENEMIES_H

#include <string>

namespace SQLite
{
class Database;
}

namespace db
{

/**
 * @brief Read every enemy json under `<assetsDir>/enemies/` and insert the
 * resulting rows into the enemies/enemy_tiers tables.
 *
 * @param database Open database connection to insert into.
 * @param assetsDir Path to the repo's assets directory (the parent of
 * `enemies/`).
 */
void seedEnemies(SQLite::Database& database, const std::string& assetsDir);

}  // namespace db

#endif
