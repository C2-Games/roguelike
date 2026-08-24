#ifndef LEVEL_H
#define LEVEL_H

#include <map>
#include <utility>

#include "objects/coordinate.h"
#include "objects/room/room.h"
#include "preload/level_loader.h"

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
   * @brief Construct a Level from an already-built room graph.
   *
   * @param meta Level-wide metadata.
   * @param rooms Every room in the level, keyed by room ID.
   * @param doorConnections Wired door-to-door adjacency, keyed by
   *                        (roomID, doorPos).
   */
  Level(LevelMeta meta, std::map<int, Room> rooms, LevelMap doorConnections);

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
  std::map<int, Room> rooms_;
  LevelMap doorConnections_;
};

#endif
