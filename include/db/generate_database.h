#ifndef DB_GENERATE_DATABASE_H
#define DB_GENERATE_DATABASE_H

#include <string>

namespace db
{

/**
 * @brief Create the game data sqlite database and seed it from the schema
 * and asset json.
 *
 * @param schemaPath Path to the sql file defining the database's tables.
 * @param assetsDir Path to the repo's assets directory (the parent of
 * `enemies/`, `weapons/`, and `levels/`).
 * @param dbPath Path the resulting sqlite database file is written to.
 */
void generateDatabase(const std::string& schemaPath,
                      const std::string& assetsDir, const std::string& dbPath);

}  // namespace db

#endif
