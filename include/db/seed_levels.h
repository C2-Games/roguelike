#ifndef DB_SEED_LEVELS_H
#define DB_SEED_LEVELS_H

#include <string>

namespace SQLite
{
class Database;
}

namespace db
{

/**
 * @brief Read every level directory under `<assetsDir>/levels/` and insert
 * the resulting rows into the levels/rooms/room_edges/room_enemy_spawns/
 * room_loot_spawns tables.
 *
 * @param database Open database connection to insert into. Must already
 * have the enemies table populated, since room_enemy_spawns.enemy_name is a
 * foreign key into it.
 * @param assetsDir Path to the repo's assets directory (the parent of
 * `levels/`).
 */
void seedLevels(SQLite::Database& database, const std::string& assetsDir);

}  // namespace db

#endif
