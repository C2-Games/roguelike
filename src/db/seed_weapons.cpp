#include "db/seed_weapons.h"

#include <SQLiteCpp/SQLiteCpp.h>

#include <cstdint>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

#include "db/json_utils.h"

namespace
{

// inserts a single weapon's base row and returns its new weapons.id.
int64_t insertWeapon(SQLite::Database& database,
                     const nlohmann::json& weaponJson)
{
  SQLite::Statement insert(
      database,
      "INSERT INTO weapons (name, type, base_damage, base_speed, base_range) "
      "VALUES (?, ?, ?, ?, ?)");
  insert.bind(1, weaponJson.at("name").get<std::string>());
  insert.bind(2, weaponJson.at("type").get<std::string>());
  insert.bind(3, weaponJson.at("damage").get<int>());
  insert.bind(4, weaponJson.at("speed").get<int>());
  insert.bind(5, weaponJson.at("range").get<int>());
  insert.exec();
  return database.getLastInsertRowid();
}

void insertWeaponTiers(SQLite::Database& database, int64_t weaponId,
                       const nlohmann::json& weaponJson)
{
  for (const auto& [tierKey, tierAttrs] : weaponJson.at("tier").items())
  {
    SQLite::Statement insert(
        database,
        "INSERT INTO weapon_tiers (weapon_id, tier, damage, speed, range) "
        "VALUES (?, ?, ?, ?, ?)");
    insert.bind(1, weaponId);
    insert.bind(2, db::parseTierKey(tierKey));
    insert.bind(3, tierAttrs.at("damage").get<int>());
    insert.bind(4, tierAttrs.at("speed").get<int>());
    insert.bind(5, tierAttrs.at("range").get<int>());
    insert.exec();
  }
}

void seedWeaponFile(SQLite::Database& database,
                    const std::filesystem::path& path)
{
  try
  {
    const nlohmann::json weaponJson = db::readJsonFile(path);

    const int64_t weaponId = insertWeapon(database, weaponJson);
    insertWeaponTiers(database, weaponId, weaponJson);
  }
  catch (const std::exception& e)
  {
    throw std::runtime_error("malformed weapon definition in " + path.string() +
                             ": " + e.what());
  }
}

}  // namespace

namespace db
{

void seedWeapons(SQLite::Database& database, const std::string& assetsDir)
{
  const std::filesystem::path weaponsDir =
      std::filesystem::path(assetsDir) / "weapons";
  db::requireDirectory(weaponsDir, "weapons");

  for (const auto& entry : std::filesystem::directory_iterator(weaponsDir))
  {
    if (!entry.is_regular_file() || entry.path().extension() != ".json")
    {
      continue;
    }
    seedWeaponFile(database, entry.path());
  }
}

}  // namespace db
