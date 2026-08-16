#include "world/map/level.h"

#include <stdexcept>
#include <utility>

#include "core/services.h"

namespace {

// Look up `adjacencyEntry`'s own edge back to `fromID` and return its
// declared direction. Doesn't assume N/S and E/W are geometric opposites —
// validates map.json's claimed symmetry instead of trusting it blindly, so a
// one-way connection (a future hand-edit mistake) fails loudly at load time
// instead of producing a door you can walk through but never back through.
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

Level::Level(const std::filesystem::path& levelDir, GameServices& services)
    : services_(services) {
  LevelConfig config = loadLevelConfig(levelDir);
  meta_ = config.meta;
  currentRoomID_ = meta_.startRoomID;
  buildFromConfig(config);
}

void Level::buildFromConfig(const LevelConfig& config) {
  for (const auto& [id, roomCfg] : config.rooms) {
    rooms_.insert(
        {id, Room::loadFromFile(id, std::filesystem::path("assets/rooms") /
                                        roomCfg.ref)});
    roomEnemyConfig_[id] = roomCfg.enemies;
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
}

const DoorConnection* Level::getDoorConnection(int roomID,
                                               Coordinate doorPos) const {
  auto it = doorConnections_.find({roomID, doorPos});
  if (it != doorConnections_.end()) return &it->second;
  return nullptr;
}
