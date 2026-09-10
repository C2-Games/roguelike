#ifndef DB_GENERATE_DATABASE_H
#define DB_GENERATE_DATABASE_H

#include <string>

namespace db
{

/**
 * @brief Create the game data sqlite database and seed it from the schema
 * and seed data sql files.
 *
 * @param schemaPath Path to the sql file defining the database's tables.
 * @param seedDataPath Path to the sql file containing the seed data insert
 * statements.
 * @param dbPath Path the resulting sqlite database file is written to.
 */
void generateDatabase(const std::string& schemaPath,
                      const std::string& seedDataPath,
                      const std::string& dbPath);

}  // namespace db

#endif
