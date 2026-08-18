#include "world/map/level.h"

#include <utility>

#include "core/services.h"

Level::Level(const std::filesystem::path& levelDir, GameServices& services,
             const EnemyCatalog& catalog)
    : services_(services)
{
  LevelConfig config = loadLevelConfig(levelDir);
  meta_ = config.meta;
  currentRoomID_ = meta_.startRoomID;
  buildFromConfig(config, catalog);
}

void Level::buildFromConfig(const LevelConfig& config,
                            const EnemyCatalog& catalog)
{
  for (const auto& [id, roomCfg] : config.rooms)
  {
    auto [it, inserted] = rooms_.insert(
        {id, Room::loadFromFile(
                 id, std::filesystem::path("assets/rooms") / roomCfg.ref)});
    it->second.ensureEnemiesSpawned(roomCfg.enemies, catalog, services_);
  }

  // each edge is authored once and wired both ways, so the graph cannot be
  // asymmetric by construction.
  for (const RoomAdjacency& edge : config.adjacency)
  {
    Coordinate fromDoor = rooms_.at(edge.fromRoom).doorAt(edge.fromDoor);
    Coordinate toDoor = rooms_.at(edge.toRoom).doorAt(edge.toDoor);
    doorConnections_[{edge.fromRoom, fromDoor}] = {edge.toRoom, toDoor};
    doorConnections_[{edge.toRoom, toDoor}] = {edge.fromRoom, fromDoor};
  }

  sealUnlinkedDoors();
}

void Level::sealUnlinkedDoors()
{
  for (auto& [id, room] : rooms_)
  {
    for (const auto& doorEntry : room.doors)
    {
      const Coordinate& door = doorEntry.second;
      if (doorConnections_.find({id, door}) == doorConnections_.end())
      {
        room.tiles[door.x][door.y] = Tile(TileType::Wall, door);
      }
    }
  }
}

const DoorConnection* Level::getDoorConnection(int roomID,
                                               Coordinate doorPos) const
{
  auto it = doorConnections_.find({roomID, doorPos});
  if (it != doorConnections_.end()) return &it->second;
  return nullptr;
}
