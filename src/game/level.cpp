#include "game/level.h"

#include <utility>

Level::Level(LevelMeta meta, std::map<int, Room> rooms,
             LevelMap doorConnections)
    : meta_(std::move(meta)),
      currentRoomID_(meta_.startRoomID),
      rooms_(std::move(rooms)),
      doorConnections_(std::move(doorConnections))
{}

Coordinate Level::transitionRoom(const DoorConnection& conn)
{
  setCurrentRoomID(conn.destRoomID);

  // place the arrival one tile inward from the destination door so the door
  // is not immediately re-triggered on the next input.
  return Room::inwardOfDoor(conn.destDoorPos);
}

const DoorConnection* Level::getDoorConnection(int roomID,
                                               Coordinate doorPos) const
{
  auto connectionEntry = doorConnections_.find({roomID, doorPos});
  if (connectionEntry != doorConnections_.end())
  {
    return &connectionEntry->second;
  }
  return nullptr;
}
