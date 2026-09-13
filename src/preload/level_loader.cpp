#include "preload/level_loader.h"

#include <SQLiteCpp/SQLiteCpp.h>

#include <cstdint>
#include <map>
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

// one room's metadata, queried from the rooms/room_enemy_spawns tables.
struct RoomConfig
{
  std::string name;
  std::string ref;  // filename under assets/rooms/
  std::vector<EnemySpawnConfig> enemies;
};

// replace a door cell and its two flanking cap cells with unbroken wall. the
// wall the door sits in runs along whichever axis has wall / cap neighbours;
// edge position isn't reliable for an interior-wall door.
void sealDoorAsWall(Room& room, Coordinate door)
{
  auto isWallish = [&](Coordinate c) {
    return Room::inBounds(c) && (room.getTileType(c) == TileType::Wall ||
                                 room.getTileType(c) == TileType::DoorCap);
  };
  const bool horizontalRun = isWallish(Coordinate{door.x - 1, door.y}) ||
                             isWallish(Coordinate{door.x + 1, door.y});
  const wchar_t wallGlyph = horizontalRun ? L'═' : L'║';

  auto sealCell = [&](Coordinate coord) {
    Tile tile(TileType::Wall, coord);
    tile.setSymbol(wallGlyph);
    room.setTile(coord, tile);
  };

  sealCell(door);

  const Coordinate flankA = horizontalRun ? Coordinate{door.x - 1, door.y}
                                          : Coordinate{door.x, door.y - 1};
  const Coordinate flankB = horizontalRun ? Coordinate{door.x + 1, door.y}
                                          : Coordinate{door.x, door.y + 1};
  for (const Coordinate& flank : {flankA, flankB})
  {
    // only overwrite an actual door cap; leave an adjacent corner or junction
    // glyph intact.
    if (Room::inBounds(flank) && room.getTileType(flank) == TileType::DoorCap)
    {
      sealCell(flank);
    }
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
      if (!roomConnections.contains(DoorConnection{id, door}))
      {
        sealDoorAsWall(room, door);
      }
    }
  }
}

}  // namespace

namespace preload
{

LevelData loadLevel(int levelID, const std::filesystem::path& dbPath,
                    const std::filesystem::path& assetsDir,
                    GameServices& services)
{
  SQLite::Database database(dbPath.string(), SQLite::OPEN_READONLY);
  std::filesystem::path roomsDir = assetsDir / "rooms";

  SQLite::Statement levelStatement(
      database,
      "SELECT name, description, room_count, start_room_id, boss_room_id "
      "FROM levels WHERE id = ?");
  levelStatement.bind(1, levelID);
  if (!levelStatement.executeStep())
  {
    throw std::runtime_error("no level with id " + std::to_string(levelID) +
                             " in " + dbPath.string());
  }
  LevelMeta meta{levelID,
                 levelStatement.getColumn(0).getString(),
                 levelStatement.getColumn(1).getString(),
                 levelStatement.getColumn(2).getInt(),
                 levelStatement.getColumn(3).getInt(),
                 levelStatement.getColumn(4).getInt()};

  // rooms.id is a synthetic global primary key, distinct from
  // local_room_id (the room's authored id); room_edges and
  // room_enemy_spawns reference the synthetic id, so it's kept here to
  // look up each room's enemy spawns.
  std::map<int, RoomConfig> rooms;
  SQLite::Statement roomsStatement(
      database,
      "SELECT id, local_room_id, name, ref FROM rooms WHERE level_id = ?");
  roomsStatement.bind(1, levelID);
  while (roomsStatement.executeStep())
  {
    const int64_t roomID = roomsStatement.getColumn(0).getInt64();
    const int localRoomID = roomsStatement.getColumn(1).getInt();

    RoomConfig config;
    config.name = roomsStatement.getColumn(2).getString();
    config.ref = roomsStatement.getColumn(3).getString();

    SQLite::Statement spawnsStatement(
        database,
        "SELECT enemy_name, tier, range_min, range_max FROM "
        "room_enemy_spawns WHERE room_id = ?");
    spawnsStatement.bind(1, roomID);
    while (spawnsStatement.executeStep())
    {
      config.enemies.push_back(
          EnemySpawnConfig{spawnsStatement.getColumn(0).getString(),
                           spawnsStatement.getColumn(1).getInt(),
                           {spawnsStatement.getColumn(2).getInt(),
                            spawnsStatement.getColumn(3).getInt()}});
    }

    rooms[localRoomID] = std::move(config);
  }

  // room_edges stores the synthetic room id; join back to rooms to recover
  // the local_room_id every other room structure keys on.
  std::vector<RoomAdjacency> adjacency;
  SQLite::Statement edgesStatement(
      database,
      "SELECT re.from_door, r1.local_room_id, re.to_door, r2.local_room_id "
      "FROM room_edges re JOIN rooms r1 ON re.from_room_id = r1.id JOIN "
      "rooms r2 ON re.to_room_id = r2.id WHERE re.level_id = ?");
  edgesStatement.bind(1, levelID);
  while (edgesStatement.executeStep())
  {
    adjacency.push_back(RoomAdjacency{edgesStatement.getColumn(1).getInt(),
                                      edgesStatement.getColumn(0).getInt(),
                                      edgesStatement.getColumn(3).getInt(),
                                      edgesStatement.getColumn(2).getInt()});
  }

  EnemyCatalog catalog(database);

  std::map<int, Room> builtRooms;
  RoomData roomData;
  for (const auto& [id, roomCfg] : rooms)
  {
    room_loader::ParsedRoom parsed =
        room_loader::loadRoom(id, roomsDir / roomCfg.ref);
    auto roomEntry = builtRooms.emplace(id, std::move(parsed.room)).first;
    roomData[id] = room_generator::generate(
        roomEntry->second, parsed.enemySpawns, parsed.lootSpawns,
        parsed.itemSpawns, roomCfg.enemies, catalog, services);
  }

  // each edge is authored once and wired both ways, so the graph cannot be
  // asymmetric by construction.
  RoomConnections roomConnections;
  for (const RoomAdjacency& edge : adjacency)
  {
    Coordinate fromDoor =
        room_loader::doorAt(builtRooms.at(edge.fromRoom), edge.fromDoor);
    Coordinate toDoor =
        room_loader::doorAt(builtRooms.at(edge.toRoom), edge.toDoor);
    roomConnections[DoorConnection{edge.fromRoom, fromDoor}] =
        DoorConnection{edge.toRoom, toDoor};
    roomConnections[DoorConnection{edge.toRoom, toDoor}] =
        DoorConnection{edge.fromRoom, fromDoor};
  }

  sealUnlinkedDoors(builtRooms, roomConnections);

  return LevelData{std::move(meta), std::move(roomConnections),
                   std::move(roomData), std::move(builtRooms)};
}

}  // namespace preload
