#include "db/generate_database.h"

#include <SQLiteCpp/SQLiteCpp.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "db/seed_enemies.h"
#include "db/seed_levels.h"
#include "db/seed_weapons.h"

namespace
{

std::string readFile(const std::string& path)
{
  std::ifstream stream(path);
  if (!stream)
  {
    throw std::runtime_error("could not open file: " + path);
  }

  std::ostringstream contents;
  contents << stream.rdbuf();
  return contents.str();
}

}  // namespace

namespace db
{

void generateDatabase(const std::string& schemaPath,
                      const std::string& assetsDir, const std::string& dbPath)
{
  // this is a derived build artifact, regenerated from scratch on every run --
  // remove any stale copy first so schema.sql's CREATE TABLE statements and
  // the seed inserts below don't collide with leftover rows/tables.
  std::filesystem::remove(dbPath);

  SQLite::Database database(dbPath,
                            SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
  database.exec("PRAGMA foreign_keys = ON;");
  database.exec(readFile(schemaPath));

  // enemies must be seeded first: room_enemy_spawns.enemy_name is a foreign
  // key into enemies.name, so seeding levels before enemies would fail every
  // enemy-spawn insert's foreign key check.
  seedEnemies(database, assetsDir);
  seedWeapons(database, assetsDir);
  seedLevels(database, assetsDir);
}

}  // namespace db
