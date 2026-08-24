#include "systems/loader/loader.h"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <map>
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "game/level.h"
#include "objects/room/room_types.h"
#include "systems/loader/enemy_spawner.h"

namespace
{

std::string trim(std::string s)
{
  auto notSpace = [](unsigned char c) { return !std::isspace(c); };
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
  s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
  return s;
}

TileType charToRoomTile(char c)
{
  if (c == '#')
  {
    return TileType::Wall;
  }
  if (c == '.')
  {
    return TileType::Floor;
  }
  if (c == 'o')
  {
    return TileType::Pillar;
  }
  if (c == ' ')
  {
    return TileType::Void;
  }
  if (c == 'E' || c == 'L')
  {
    return TileType::Floor;
  }
  if (c >= '0' && c <= '9')
  {
    return TileType::Door;
  }
  std::ostringstream oss;
  oss << "Unrecognized room-file character: '" << c << "' (0x" << std::hex
      << static_cast<int>(static_cast<unsigned char>(c)) << ")";
  throw std::runtime_error(oss.str());
}

enum class SpawnKind : std::uint8_t
{
  None,
  Enemy,
  LootOrItem
};

SpawnKind charToSpawnKind(char c)
{
  switch (c)
  {
    case 'E':
      return SpawnKind::Enemy;
    case 'L':
      return SpawnKind::LootOrItem;
    default:
      return SpawnKind::None;
  }
}

// parse the '@key: value' header block, leaving `in` positioned at the start
// of the grid. blank lines before the grid are permitted so authors can
// space out the header visually.
void parseRoomHeader(std::ifstream& in, Room& room,
                     const std::filesystem::path& path)
{
  std::string line;
  std::streampos gridStart = in.tellg();
  while (std::getline(in, line))
  {
    std::string trimmed = trim(line);
    if (trimmed.empty())
    {
      gridStart = in.tellg();
      continue;
    }
    if (trimmed[0] != '@')
    {
      // rewind — this line belongs to the grid.
      in.clear();
      in.seekg(gridStart);
      return;
    }

    // parse "@key: value".
    auto colon = trimmed.find(':');
    if (colon == std::string::npos)
    {
      throw std::runtime_error("Malformed header (no colon) in " +
                               path.string() + ": " + trimmed);
    }
    std::string key = trim(trimmed.substr(1, colon - 1));
    std::string value = trim(trimmed.substr(colon + 1));

    if (key == "name")
    {
      room.name = value;
    }
    // other keys (levels, author, ...) are parsed by the library layer or
    // silently ignored here for forward compatibility.

    gridStart = in.tellg();
  }
}

// parse the ASCII grid following the header, populating tiles, doors, and
// spawn points. throws if the row count doesn't match Room::HEIGHT.
void parseRoomGrid(std::ifstream& in, Room& room,
                   const std::filesystem::path& path)
{
  std::string line;
  int y = 0;
  while (std::getline(in, line))
  {
    // strip trailing CR so CRLF-terminated files (common on Windows editors)
    // parse the same as LF.
    if (!line.empty() && line.back() == '\r')
    {
      line.pop_back();
    }

    if (y >= Room::HEIGHT)
    {
      throw std::runtime_error("Too many grid rows in " + path.string() +
                               " (expected " + std::to_string(Room::HEIGHT) +
                               ")");
    }
    // pad short lines with spaces (Void) but reject over-long lines to catch
    // authoring mistakes.
    if (static_cast<int>(line.size()) > Room::WIDTH)
    {
      throw std::runtime_error(
          "Row " + std::to_string(y) + " in " + path.string() +
          " is too wide: " + std::to_string(line.size()) + " chars (expected " +
          std::to_string(Room::WIDTH) + ")");
    }
    if (static_cast<int>(line.size()) < Room::WIDTH)
    {
      line.append(Room::WIDTH - line.size(), ' ');
    }

    for (int x = 0; x < Room::WIDTH; ++x)
    {
      char c = line[x];
      TileType type = charToRoomTile(c);
      room.tiles[x][y] = Tile(type, Coordinate(x, y));
      if (type == TileType::Door)
      {
        DoorNumber label = c - '0';
        const bool inserted =
            room.doors.emplace(label, Coordinate{x, y}).second;
        if (!inserted)
        {
          throw std::runtime_error("Duplicate door label '" +
                                   std::string(1, c) + "' in " + path.string());
        }
      }
      switch (charToSpawnKind(c))
      {
        case SpawnKind::Enemy:
          room.enemySpawns.push_back(Coordinate(x, y));
          break;
        case SpawnKind::LootOrItem:
          room.lootSpawns.push_back(Coordinate(x, y));
          room.itemSpawns.push_back(Coordinate(x, y));
          break;
        case SpawnKind::None:
          break;
      }
    }
    ++y;
  }

  if (y != Room::HEIGHT)
  {
    throw std::runtime_error("Not enough grid rows in " + path.string() +
                             ": got " + std::to_string(y) + ", expected " +
                             std::to_string(Room::HEIGHT));
  }
}

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
                       const LevelMap& doorConnections)
{
  for (auto& [id, room] : rooms)
  {
    for (const auto& doorEntry : room.doors)
    {
      const Coordinate& door = doorEntry.second;
      if (doorConnections.find({id, door}) == doorConnections.end())
      {
        room.tiles[door.x][door.y] = Tile(TileType::Wall, door);
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

Loader::Loader(const std::filesystem::path& assetsDir)
    : assetsDir_(assetsDir), catalog_(enemyCatalogDir())
{}

Room Loader::loadRoom(int roomID, const std::filesystem::path& path) const
{
  std::ifstream in(path);
  if (!in)
  {
    throw std::runtime_error("Could not open room file: " + path.string());
  }

  Room room(roomID);
  parseRoomHeader(in, room, path);
  parseRoomGrid(in, room, path);

  for (const auto& [number, doorPos] : room.doors)
  {
    Coordinate entry = Room::inwardOfDoor(doorPos);
    room.tiles[entry.x][entry.y] = Tile(TileType::EntryWay, entry);
  }

  return room;
}

Level Loader::loadLevel(const std::filesystem::path& levelDir,
                        GameServices& services) const
{
  LevelConfig config = loadLevelConfig(levelDir);

  std::map<int, Room> rooms;
  for (const auto& [id, roomCfg] : config.rooms)
  {
    auto roomEntry =
        rooms.insert({id, loadRoom(id, roomsDir() / roomCfg.ref)}).first;
    enemy_spawner::ensureSpawned(roomEntry->second, roomCfg.enemies, catalog_,
                                 services);
  }

  // each edge is authored once and wired both ways, so the graph cannot be
  // asymmetric by construction.
  LevelMap doorConnections;
  for (const RoomAdjacency& edge : config.adjacency)
  {
    Coordinate fromDoor = rooms.at(edge.fromRoom).doorAt(edge.fromDoor);
    Coordinate toDoor = rooms.at(edge.toRoom).doorAt(edge.toDoor);
    doorConnections[{edge.fromRoom, fromDoor}] = {edge.toRoom, toDoor};
    doorConnections[{edge.toRoom, toDoor}] = {edge.fromRoom, fromDoor};
  }

  sealUnlinkedDoors(rooms, doorConnections);

  return Level(config.meta, std::move(rooms), std::move(doorConnections));
}
