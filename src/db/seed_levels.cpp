#include "db/seed_levels.h"

#include <SQLiteCpp/SQLiteCpp.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <iterator>
#include <map>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <vector>

#include "db/json_utils.h"

namespace
{

// binds the level's own authored id explicitly (rather than letting sqlite
// autoincrement) so rooms/room_edges can reference level_id by that same
// value.
int64_t insertLevel(SQLite::Database& database, const nlohmann::json& levelJson)
{
  SQLite::Statement insert(database,
                           "INSERT INTO levels (id, name, description, "
                           "room_count, start_room_id, boss_room_id) "
                           "VALUES (?, ?, ?, ?, ?, ?)");
  const int64_t levelId = levelJson.at("id").get<int64_t>();
  insert.bind(1, levelId);
  insert.bind(2, levelJson.at("name").get<std::string>());
  insert.bind(3, levelJson.at("description").get<std::string>());
  insert.bind(4, levelJson.at("roomCount").get<int>());
  insert.bind(5, levelJson.at("startRoomID").get<int>());
  insert.bind(6, levelJson.at("bossRoomID").get<int>());
  insert.exec();
  return levelId;
}

// inserts one room's row and returns the synthetic rooms.id sqlite assigned,
// which room_enemy_spawns/room_loot_spawns/room_edges reference.
int64_t insertRoom(SQLite::Database& database, int64_t levelId,
                   const nlohmann::json& roomJson)
{
  SQLite::Statement insert(database,
                           "INSERT INTO rooms (level_id, local_room_id, name, "
                           "ref) VALUES (?, ?, ?, ?)");
  insert.bind(1, levelId);
  insert.bind(2, roomJson.at("id").get<int>());
  insert.bind(3, roomJson.at("name").get<std::string>());
  insert.bind(4, roomJson.at("ref").get<std::string>());
  insert.exec();
  return database.getLastInsertRowid();
}

void insertRoomEnemySpawns(SQLite::Database& database, int64_t roomId,
                           const nlohmann::json& roomJson)
{
  for (const auto& enemyJson : roomJson.at("enemies"))
  {
    const nlohmann::json& range = enemyJson.at("range");
    SQLite::Statement insert(database,
                             "INSERT INTO room_enemy_spawns (room_id, "
                             "enemy_name, class, tier, range_min, range_max) "
                             "VALUES (?, ?, ?, ?, ?, ?)");
    insert.bind(1, roomId);
    insert.bind(2, enemyJson.at("name").get<std::string>());
    insert.bind(3, enemyJson.at("class").get<std::string>());
    insert.bind(4, enemyJson.at("tier").get<int>());
    insert.bind(5, range.at(0).get<int>());
    insert.bind(6, range.at(1).get<int>());
    insert.exec();
  }
}

void insertRoomLootSpawns(SQLite::Database& database, int64_t roomId,
                          const nlohmann::json& roomJson)
{
  for (const auto& lootJson : roomJson.at("loot"))
  {
    const nlohmann::json& range = lootJson.at("range");
    SQLite::Statement insert(database,
                             "INSERT INTO room_loot_spawns (room_id, "
                             "loot_name, class, tier, range_min, range_max) "
                             "VALUES (?, ?, ?, ?, ?, ?)");
    insert.bind(1, roomId);
    insert.bind(2, lootJson.at("name").get<std::string>());
    insert.bind(3, lootJson.at("class").get<std::string>());
    insert.bind(4, lootJson.at("tier").get<int>());
    insert.bind(5, range.at(0).get<int>());
    insert.bind(6, range.at(1).get<int>());
    insert.exec();
  }
}

void insertRoomEdges(SQLite::Database& database, int64_t levelId,
                     const nlohmann::json& edgesJson,
                     const std::map<int, int64_t>& roomIdByLocalId)
{
  for (const auto& edgeJson : edgesJson)
  {
    const nlohmann::json& from = edgeJson.at("from");
    const nlohmann::json& to = edgeJson.at("to");
    SQLite::Statement insert(database,
                             "INSERT INTO room_edges (level_id, "
                             "from_room_id, from_door, to_room_id, to_door) "
                             "VALUES (?, ?, ?, ?, ?)");
    insert.bind(1, levelId);
    insert.bind(2, roomIdByLocalId.at(from.at("room").get<int>()));
    insert.bind(3, from.at("door").get<int>());
    insert.bind(4, roomIdByLocalId.at(to.at("room").get<int>()));
    insert.bind(5, to.at("door").get<int>());
    insert.exec();
  }
}

// throws if map.json's room-id count does not match level.json's roomCount.
void validateRoomCount(const nlohmann::json& levelJson,
                       const nlohmann::json& mapJson,
                       const std::filesystem::path& levelDir)
{
  const int roomCount = levelJson.at("roomCount").get<int>();
  const std::size_t mapRoomCount = mapJson.at("rooms").size();
  if (static_cast<int>(mapRoomCount) != roomCount)
  {
    throw std::runtime_error(
        "level.json roomCount (" + std::to_string(roomCount) +
        ") does not match map.json room count (" +
        std::to_string(mapRoomCount) + ") in " + levelDir.string());
  }
}

// throws if an edge references a room id not present in map.json's own room
// list.
void validateEdgeRoomsKnown(const nlohmann::json& mapJson,
                            const std::filesystem::path& levelDir)
{
  const nlohmann::json& roomsJson = mapJson.at("rooms");
  std::vector<int> roomIDs;
  roomIDs.reserve(roomsJson.size());
  std::transform(
      roomsJson.begin(), roomsJson.end(), std::back_inserter(roomIDs),
      [](const nlohmann::json& roomIdJson) { return roomIdJson.get<int>(); });

  auto requireKnownRoom = [&roomIDs, &levelDir](int roomID) {
    if (std::find(roomIDs.begin(), roomIDs.end(), roomID) == roomIDs.end())
    {
      throw std::runtime_error(
          "map.json edge references room " + std::to_string(roomID) +
          ", which is not in its room list, in " + levelDir.string());
    }
  };
  for (const auto& edgeJson : mapJson.at("edges"))
  {
    requireKnownRoom(edgeJson.at("from").at("room").get<int>());
    requireKnownRoom(edgeJson.at("to").at("room").get<int>());
  }
}

// throws if a room_N.json's own id field does not match its filename-derived
// expected id.
void validateRoomId(const nlohmann::json& roomJson, int expectedId)
{
  const int actualId = roomJson.at("id").get<int>();
  if (actualId != expectedId)
  {
    throw std::runtime_error("id " + std::to_string(actualId) +
                             " does not match expected id " +
                             std::to_string(expectedId));
  }
}

void seedLevelDir(SQLite::Database& database,
                  const std::filesystem::path& levelDir)
{
  try
  {
    const nlohmann::json levelJson = db::readJsonFile(levelDir / "level.json");
    const nlohmann::json mapJson = db::readJsonFile(levelDir / "map.json");

    validateRoomCount(levelJson, mapJson, levelDir);

    const int64_t levelId = insertLevel(database, levelJson);

    std::map<int, int64_t> roomIdByLocalId;
    for (const auto& localRoomIdJson : mapJson.at("rooms"))
    {
      const int localRoomId = localRoomIdJson.get<int>();
      const std::filesystem::path roomPath =
          levelDir / ("room_" + std::to_string(localRoomId) + ".json");
      const nlohmann::json roomJson = db::readJsonFile(roomPath);
      validateRoomId(roomJson, localRoomId);

      const int64_t roomId = insertRoom(database, levelId, roomJson);
      roomIdByLocalId[localRoomId] = roomId;
      insertRoomEnemySpawns(database, roomId, roomJson);
      insertRoomLootSpawns(database, roomId, roomJson);
    }

    validateEdgeRoomsKnown(mapJson, levelDir);
    insertRoomEdges(database, levelId, mapJson.at("edges"), roomIdByLocalId);
  }
  catch (const std::exception& e)
  {
    throw std::runtime_error("malformed level definition in " +
                             levelDir.string() + ": " + e.what());
  }
}

}  // namespace

namespace db
{

void seedLevels(SQLite::Database& database, const std::string& assetsDir)
{
  const std::filesystem::path levelsDir =
      std::filesystem::path(assetsDir) / "levels";
  db::requireDirectory(levelsDir, "levels");

  for (const auto& entry : std::filesystem::directory_iterator(levelsDir))
  {
    if (!entry.is_directory() ||
        entry.path().filename().string().rfind("level_", 0) != 0)
    {
      continue;
    }
    seedLevelDir(database, entry.path());
  }
}

}  // namespace db
