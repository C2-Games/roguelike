#include "db/seed_enemies.h"

#include <SQLiteCpp/SQLiteCpp.h>

#include <cstdint>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <stdexcept>

#include "db/json_utils.h"

namespace
{

void insertTier(SQLite::Database& database, int64_t enemyId,
                const std::string& tierKey,
                const nlohmann::json& tierAttributes)
{
  const int tier = db::parseTierKey(tierKey);
  const auto& fov = tierAttributes.at("fov");

  SQLite::Statement insertTierStatement(
      database,
      "INSERT INTO enemy_tiers (enemy_id, tier, health, damage_amount, "
      "damage_type, fov_x, "
      "fov_y, chase, speed, extra_drops) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, "
      "?)");
  insertTierStatement.bind(1, enemyId);
  insertTierStatement.bind(2, tier);
  insertTierStatement.bind(3, tierAttributes.at("health").get<int>());
  insertTierStatement.bind(4,
                           tierAttributes.at("damage").at("amount").get<int>());
  insertTierStatement.bind(
      5, tierAttributes.at("damage").at("type").get<std::string>());
  insertTierStatement.bind(6, fov.at(0).get<int>());
  insertTierStatement.bind(7, fov.at(1).get<int>());
  insertTierStatement.bind(8, tierAttributes.at("chase").get<int>());
  insertTierStatement.bind(9, tierAttributes.at("speed").get<int>());

  const nlohmann::json& extraDrops = tierAttributes.at("extra_drops");
  if (extraDrops.is_null())
  {
    insertTierStatement.bind(10);
  }
  else
  {
    insertTierStatement.bind(10, extraDrops.get<std::string>());
  }

  insertTierStatement.exec();
}

void seedEnemyFile(SQLite::Database& database,
                   const std::filesystem::path& path)
{
  try
  {
    const nlohmann::json enemyJson = db::readJsonFile(path);

    const std::string name = enemyJson.at("name").get<std::string>();
    const std::string enemyClass = enemyJson.at("class").get<std::string>();
    const nlohmann::json& symbolJson = enemyJson.at("symbol");

    SQLite::Statement insertEnemyStatement(
        database, "INSERT INTO enemies (name, class, symbol) VALUES (?, ?, ?)");
    insertEnemyStatement.bind(1, name);
    insertEnemyStatement.bind(2, enemyClass);
    insertEnemyStatement.bind(3, symbolJson.dump());
    insertEnemyStatement.exec();

    const int64_t enemyId = database.getLastInsertRowid();

    for (const auto& [tierKey, tierAttributes] :
         enemyJson.at("attributes").items())
    {
      insertTier(database, enemyId, tierKey, tierAttributes);
    }
  }
  catch (const std::exception& e)
  {
    throw std::runtime_error("malformed enemy definition in " + path.string() +
                             ": " + e.what());
  }
}

}  // namespace

namespace db
{

void seedEnemies(SQLite::Database& database, const std::string& assetsDir)
{
  const std::filesystem::path enemiesDir =
      std::filesystem::path(assetsDir) / "enemies";
  db::requireDirectory(enemiesDir, "enemy assets");

  for (const auto& entry : std::filesystem::directory_iterator(enemiesDir))
  {
    if (!entry.is_regular_file() || entry.path().extension() != ".json")
    {
      continue;
    }
    seedEnemyFile(database, entry.path());
  }
}

}  // namespace db
