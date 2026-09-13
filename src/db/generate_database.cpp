#include "db/generate_database.h"

#include <SQLiteCpp/SQLiteCpp.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

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
                      const std::string& seedDataPath,
                      const std::string& dbPath)
{
  // this is a derived build artifact, regenerated from scratch on every run --
  // remove any stale copy first so schema.sql's CREATE TABLE statements and
  // the seed inserts below don't collide with leftover rows/tables.
  std::filesystem::remove(dbPath);

  SQLite::Database database(dbPath,
                            SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
  database.exec("PRAGMA foreign_keys = ON;");
  database.exec(readFile(schemaPath));
  database.exec(readFile(seedDataPath));
}

}  // namespace db
