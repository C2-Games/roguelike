#ifndef LEVEL_H
#define LEVEL_H

#include <filesystem>
#include <map>
#include <utility>
#include <vector>

#include "core/coordinate.h"
#include "world/map/level_config.h"
#include "world/map/room.h"

struct GameServices;

struct DoorConnection {
  int destRoomID;
  Coordinate destDoorPos;
};

class Level {
 public:
  /**
   * @brief Load a level directory (level.json, map.json, and every room's
   * JSON metadata + referenced .txt template) and wire the room graph.
   *
   * @param levelDir Directory containing the level's config files.
   * @param services Shared services; stored by reference; must outlive
   *                 the Level.
   */
  Level(const std::filesystem::path& levelDir, GameServices& services);

  /**
   * @brief Look up the door connection at (roomID, doorPos).
   *
   * @return Pointer to the connection, or nullptr if the door is unlinked.
   */
  const DoorConnection* getDoorConnection(int roomID, Coordinate doorPos) const;

  /** @brief Total number of rooms in the level. */
  int getRoomCount() const { return meta_.roomCount; }

  /** @brief ID of the room the player is currently in. */
  int getCurrentRoomID() const { return currentRoomID_; }

  /** @brief Set the current-room cursor (called on door transitions). */
  void setCurrentRoomID(int id) { currentRoomID_ = id; }

  /** @brief Const access to the current room. */
  const Room& getCurrentRoom() const { return rooms_.at(currentRoomID_); }

  /**
   * @brief Mutable access to the current room, for systems that update
   * per-frame Room state (e.g. visibility bitmaps, enemy state).
   */
  Room& getCurrentRoom() { return rooms_.at(currentRoomID_); }

  /** @brief Const access to any room by ID. */
  const Room& getRoom(int roomID) const { return rooms_.at(roomID); }

  /** @brief The authored enemy spawn table for a room, by ID. */
  const std::vector<EnemySpawnConfig>& getRoomEnemyConfig(int roomID) const {
    return roomEnemyConfig_.at(roomID);
  }

 private:
  LevelMeta meta_;
  int currentRoomID_;
  GameServices& services_;
  std::map<int, Room> rooms_;
  std::map<std::pair<int, Coordinate>, DoorConnection> doorConnections_;
  std::map<int, std::vector<EnemySpawnConfig>> roomEnemyConfig_;

  /** @brief Populate rooms_, roomEnemyConfig_, and doorConnections_ from a
   *  parsed LevelConfig. Called from the constructor; not intended to be
   *  re-run. */
  void buildFromConfig(const LevelConfig& config);
};

#endif
