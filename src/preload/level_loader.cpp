#include "preload/level_loader.h"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <map>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "game/level_data.h"
#include "objects/tiles/tile.h"
#include "objects/tiles/tile_type.h"
#include "preload/enemy_catalog.h"
#include "preload/room_generator.h"
#include "preload/room_loader.h"

namespace
{

// one door-to-door link between two rooms, currently bidirectional
// (fromRoom->toRoom and toRoom->fromRoom).
struct RoomAdjacency
{
  int fromRoom;
  DoorNumber fromDoor;
  int toRoom;
  DoorNumber toDoor;
};

// one room's metadata, parsed from room_<id>.json.
struct RoomConfig
{
  int id;
  std::string name;
  std::string ref;  // filename under assets/rooms/
  std::vector<EnemySpawnConfig> enemies;
};

// fully parsed contents of a level directory.
struct LevelConfig
{
  LevelMeta meta;
  std::vector<RoomAdjacency> adjacency;  // from map.json
  std::map<int, RoomConfig> rooms;       // id -> per-room config
};

nlohmann::json readJson(const std::filesystem::path& path)
{
  std::ifstream in(path);
  if (!in)
  {
    throw std::runtime_error("could not open file: " + path.string());
  }

  nlohmann::json j;
  in >> j;
  return j;
}

LevelMeta parseLevelMeta(const nlohmann::json& j)
{
  return LevelMeta{
      j.at("id").get<int>(),
      j.at("name").get<std::string>(),
      j.at("description").get<std::string>(),
      j.at("roomCount").get<int>(),
      j.at("startRoomID").get<int>(),
      j.at("bossRoomID").get<int>(),
  };
}

LevelMeta loadLevelMeta(const std::filesystem::path& path)
{
  try
  {
    return parseLevelMeta(readJson(path));
  }
  catch (const std::exception& e)
  {
    throw std::runtime_error("malformed level.json at " + path.string() + ": " +
                             e.what());
  }
}

struct MapData
{
  std::vector<int> roomIDs;
  std::vector<RoomAdjacency> adjacency;
};

MapData parseMap(const nlohmann::json& j)
{
  MapData data;

  const auto& rooms = j.at("rooms");
  data.roomIDs.reserve(rooms.size());
  std::transform(rooms.begin(), rooms.end(), std::back_inserter(data.roomIDs),
                 [](const nlohmann::json& id) { return id.get<int>(); });

  const auto& edges = j.at("edges");
  data.adjacency.reserve(edges.size());
  std::transform(edges.begin(), edges.end(), std::back_inserter(data.adjacency),
                 [](const nlohmann::json& edge) {
                   const auto& from = edge.at("from");
                   const auto& to = edge.at("to");
                   return RoomAdjacency{
                       from.at("room").get<int>(),
                       from.at("door").get<DoorNumber>(),
                       to.at("room").get<int>(),
                       to.at("door").get<DoorNumber>(),
                   };
                 });

  return data;
}

MapData loadMap(const std::filesystem::path& path)
{
  try
  {
    return parseMap(readJson(path));
  }
  catch (const std::exception& e)
  {
    throw std::runtime_error("malformed map.json at " + path.string() + ": " +
                             e.what());
  }
}

EnemySpawnConfig parseEnemySpawnConfig(const nlohmann::json& j)
{
  const nlohmann::json& range = j.at("range");
  if (!range.is_array() || range.size() != 2)
  {
    throw std::runtime_error("enemy entry '" + j.at("name").get<std::string>() +
                             "' needs a two-element range");
  }

  return EnemySpawnConfig{
      j.at("name").get<std::string>(),
      j.at("tier").get<int>(),
      {range[0].get<int>(), range[1].get<int>()},
  };
}

RoomConfig parseRoomConfig(const nlohmann::json& j)
{
  RoomConfig config;
  config.id = j.at("id").get<int>();
  config.name = j.at("name").get<std::string>();
  config.ref = j.at("ref").get<std::string>();

  if (j.contains("enemies"))
  {
    for (const auto& entry : j.at("enemies"))
    {
      config.enemies.push_back(parseEnemySpawnConfig(entry));
    }
  }
  return config;
}

RoomConfig loadRoomConfig(const std::filesystem::path& path, int expectedId)
{
  try
  {
    RoomConfig config = parseRoomConfig(readJson(path));
    if (config.id != expectedId)
    {
      throw std::runtime_error("id " + std::to_string(config.id) +
                               " does not match expected id " +
                               std::to_string(expectedId));
    }
    return config;
  }
  catch (const std::exception& e)
  {
    throw std::runtime_error("malformed room config at " + path.string() +
                             ": " + e.what());
  }
}

void sealUnlinkedDoors(std::map<int, Room>& rooms,
                       const RoomConnections& roomConnections)
{
  for (auto& [id, room] : rooms)
  {
    for (const auto& doorEntry : room.getDoors())
    {
      const Coordinate& door = doorEntry.second;
      if (roomConnections.find(DoorConnection{id, door}) ==
          roomConnections.end())
      {
        room.setTile(door, Tile(TileType::Wall, door));
      }
    }
  }
}

// load and cross-validate an entire level directory (level.json, map.json,
// and every room_<id>.json map.json references). throws std::runtime_error
// on any missing file, malformed JSON, or cross-reference mismatch (id
// mismatches, room-count mismatches).
LevelConfig loadLevelConfig(const std::filesystem::path& levelDir)
{
  LevelConfig config;
  config.meta = loadLevelMeta(levelDir / "level.json");

  MapData map = loadMap(levelDir / "map.json");
  config.adjacency = std::move(map.adjacency);

  for (int id : map.roomIDs)
  {
    std::filesystem::path roomPath =
        levelDir / ("room_" + std::to_string(id) + ".json");
    config.rooms[id] = loadRoomConfig(roomPath, id);
  }

  if (static_cast<int>(map.roomIDs.size()) != config.meta.roomCount)
  {
    throw std::runtime_error(
        "level.json roomCount (" + std::to_string(config.meta.roomCount) +
        ") does not match map.json room count (" +
        std::to_string(map.roomIDs.size()) + ") in " + levelDir.string());
  }

  auto requireKnownRoom = [&config, &levelDir](int roomID) {
    if (config.rooms.find(roomID) == config.rooms.end())
    {
      throw std::runtime_error(
          "map.json edge references room " + std::to_string(roomID) +
          ", which is not in its room list, in " + levelDir.string());
    }
  };
  for (const RoomAdjacency& edge : config.adjacency)
  {
    requireKnownRoom(edge.fromRoom);
    requireKnownRoom(edge.toRoom);
  }

  return config;
}

}  // namespace

namespace preload
{

LevelData loadLevel(const std::filesystem::path& levelDir,
                    const std::filesystem::path& assetsDir,
                    GameServices& services)
{
  EnemyCatalog catalog(assetsDir / "enemies");
  std::filesystem::path roomsDir = assetsDir / "rooms";

  LevelConfig config = loadLevelConfig(levelDir);

  std::map<int, Room> rooms;
  RoomData roomData;
  for (const auto& [id, roomCfg] : config.rooms)
  {
    room_loader::ParsedRoom parsed =
        room_loader::loadRoom(id, roomsDir / roomCfg.ref);
    auto roomEntry = rooms.emplace(id, std::move(parsed.room)).first;
    roomData[id] = room_generator::generate(
        roomEntry->second, parsed.enemySpawns, parsed.lootSpawns,
        parsed.itemSpawns, roomCfg.enemies, catalog, services);
  }

  // each edge is authored once and wired both ways, so the graph cannot be
  // asymmetric by construction.
  RoomConnections roomConnections;
  for (const RoomAdjacency& edge : config.adjacency)
  {
    Coordinate fromDoor =
        room_loader::doorAt(rooms.at(edge.fromRoom), edge.fromDoor);
    Coordinate toDoor = room_loader::doorAt(rooms.at(edge.toRoom), edge.toDoor);
    roomConnections[DoorConnection{edge.fromRoom, fromDoor}] =
        DoorConnection{edge.toRoom, toDoor};
    roomConnections[DoorConnection{edge.toRoom, toDoor}] =
        DoorConnection{edge.fromRoom, fromDoor};
  }

  sealUnlinkedDoors(rooms, roomConnections);

  return LevelData{std::move(config.meta), std::move(roomConnections),
                   std::move(roomData), std::move(rooms)};
}

}  // namespace preload
