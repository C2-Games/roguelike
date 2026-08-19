#ifndef LEVEL_H
#define LEVEL_H

#include <filesystem>
#include <map>
#include <utility>

#include "core/coordinate.h"
#include "world/map/level_config.h"
#include "world/map/room.h"

struct GameServices;
class EnemyCatalog;

struct DoorConnection
{
  int destRoomID;
  Coordinate destDoorPos;
};

using LevelMap = std::map<std::pair<int, Coordinate>, DoorConnection>;

class Level
{
 public:
  /**
   * @brief Load a level directory (level.json, map.json, and every room's
   * JSON metadata + referenced .txt template), wire the room graph, and
   * spawn every room's enemies.
   *
   * @param levelDir Directory containing the level's config files.
   * @param services Shared services; stored by reference; must outlive
   *                 the Level.
   * @param catalog  Resolves each room's spawn-table entries to stats.
   */
  Level(const std::filesystem::path& levelDir, GameServices& services,
        const EnemyCatalog& catalog);

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

  /**
   * @brief Enter the room on the far side of `conn`.
   *
   * @param conn Door connection describing the destination.
   * @return The tile one step inward from the destination door, so whoever
   * walked through does not immediately re-trigger it.
   */
  Coordinate transitionRoom(const DoorConnection& conn);

  /** @brief Const access to the current room. */
  const Room& getCurrentRoom() const { return rooms_.at(currentRoomID_); }

  /**
   * @brief Mutable access to the current room, for systems that update
   * per-frame Room state (e.g. visibility bitmaps, enemy state).
   */
  Room& getCurrentRoom() { return rooms_.at(currentRoomID_); }

  /** @brief Const access to any room by ID. */
  const Room& getRoom(int roomID) const { return rooms_.at(roomID); }

 private:
  LevelMeta meta_;
  int currentRoomID_;
  GameServices& services_;
  std::map<int, Room> rooms_;
  LevelMap doorConnections_;

  /** @brief Populate rooms_ and doorConnections_ from a parsed LevelConfig,
   *  spawn each room's enemies, and seal any of its doors left unlinked by
   *  this level's adjacency back to Wall tiles.
   */
  void buildFromConfig(const LevelConfig& config, const EnemyCatalog& catalog);

  /** @brief convert unlinked doors to Wall tiles */
  void sealUnlinkedDoors();
};

#endif
