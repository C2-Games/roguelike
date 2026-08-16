#include "world/map/level.h"

#include <stdexcept>
#include <utility>

#include "core/services.h"

namespace {

// Look up `adjacencyEntry`'s own edge back to `fromID` and return its
// declared direction.
Direction findReverseDirection(const std::vector<RoomEdge>& adjacencyEntry,
                               int fromID) {
  for (const RoomEdge& edge : adjacencyEntry) {
    if (edge.to == fromID) return edge.direction;
  }
  throw std::runtime_error(
      "room has no reverse connection back to room " + std::to_string(fromID) +
      " — map.json's adjacency is not symmetric");
}

}  // namespace

Level::Level(const std::filesystem::path& levelDir, GameServices& services,
            const EnemyCatalog& catalog)
    : services_(services) {
  LevelConfig config = loadLevelConfig(levelDir);
  meta_ = config.meta;
  currentRoomID_ = meta_.startRoomID;
  buildFromConfig(config, catalog);
}

void Level::buildFromConfig(const LevelConfig& config,
                            const EnemyCatalog& catalog) {
  for (const auto& [id, roomCfg] : config.rooms) {
    auto [it, inserted] = rooms_.insert(
        {id, Room::loadFromFile(id, std::filesystem::path("assets/rooms") /
                                        roomCfg.ref)});
    roomEnemyConfig_[id] = roomCfg.enemies;
    it->second.ensureEnemiesSpawned(roomCfg.enemies, catalog, services_);
  }

  for (const auto& [fromID, edges] : config.adjacency) {
    const Room& fromRoom = rooms_.at(fromID);
    for (const RoomEdge& edge : edges) {
      const Room& toRoom = rooms_.at(edge.to);
      Coordinate fromDoor = resolveDoorForDirection(fromRoom, edge.direction);
      Direction reverseDir =
          findReverseDirection(config.adjacency.at(edge.to), fromID);
      Coordinate toDoor = resolveDoorForDirection(toRoom, reverseDir);
      doorConnections_[{fromID, fromDoor}] = {edge.to, toDoor};
    }
  }

  sealUnlinkedDoors();
}

void Level::sealUnlinkedDoors() {
  for (auto& [id, room] : rooms_) {
    for (const Coordinate& door : room.doorPositions) {
      if (doorConnections_.find({id, door}) == doorConnections_.end()) {
        room.tiles[door.x][door.y] = Tile(TileType::Wall, door);
      }
    }
  }
}

const DoorConnection* Level::getDoorConnection(int roomID,
                                               Coordinate doorPos) const {
  auto it = doorConnections_.find({roomID, doorPos});
  if (it != doorConnections_.end()) return &it->second;
  return nullptr;
}
