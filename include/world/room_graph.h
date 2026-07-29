#ifndef ROOM_GRAPH_H
#define ROOM_GRAPH_H

#include <map>
#include <utility>
#include <vector>

#include "core/coordinate.h"
#include "world/room.h"

struct GameServices;

/**
 * @brief Describes a one-way portal from a door tile to a destination room.
 *
 * Two DoorConnection records (one per direction) form a bidirectional link
 * between rooms. Stored in RoomGraph keyed by (roomID, doorPos).
 */
struct DoorConnection {
  int destRoomID;          ///< ID of the room the door leads to.
  Coordinate destDoorPos;  ///< Position of the matching door in that room.
};

/**
 * @brief Owns the collection of rooms and the door graph between them.
 *
 * On construction it loads `roomCount` randomly-picked rooms from
 * assets/rooms/ and wires them into a linear chain of bidirectional door
 * connections. The current-room index is a mutable cursor: callers move it
 * via setCurrentRoomID when the player transitions through a door.
 *
 */
class RoomGraph {
 public:
  /**
   * @brief Construct and immediately load / wire the rooms.
   *
   * @param roomCount How many rooms to include in the chain.
   * @param services  Shared services; RNG used to pick rooms from the
   *                  authored library. Stored by reference; must outlive
   *                  the RoomGraph.
   */
  RoomGraph(int roomCount, GameServices& services);

  /**
   * @brief Look up the door connection at (roomID, doorPos).
   *
   * @return Pointer to the connection, or nullptr if the door is unlinked.
   */
  const DoorConnection* getDoorConnection(int roomID,
                                          Coordinate doorPos) const;

  /** @brief Total number of rooms in the chain. */
  int getRoomCount() const { return roomCount_; }

  /** @brief ID of the room the player is currently in. */
  int getCurrentRoomID() const { return currentRoomID_; }

  /** @brief Set the current-room cursor (called on door transitions). */
  void setCurrentRoomID(int id) { currentRoomID_ = id; }

  /** @brief Const access to the current room. */
  const Room& getCurrentRoom() const { return rooms_.at(currentRoomID_); }

  /**
   * @brief Mutable access to the current room, for systems that update
   * per-frame Room state (e.g. visibility bitmaps).
   */
  Room& getCurrentRoom() { return rooms_.at(currentRoomID_); }

  /** @brief Const access to any room by ID. */
  const Room& getRoom(int roomID) const { return rooms_.at(roomID); }

 private:
  int roomCount_;
  int currentRoomID_ = 0;
  GameServices& services_;
  std::map<int, Room> rooms_;                     ///< All rooms keyed by ID.
  std::vector<std::vector<int>> adjacency_;       ///< Room adjacency list.
  std::map<std::pair<int, Coordinate>, DoorConnection>
      doorConnections_;  ///< (roomID, doorPos) → destination.

  /**
   * @brief Load rooms from the library and wire the door chain. Called
   * from the constructor; not intended to be re-run.
   */
  void generate();

  /** @brief Insert a fully-parsed room into the graph. */
  void addRoom(Room room);
};

#endif
