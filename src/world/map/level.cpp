#include "world/map/level.h"

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

  // Place the arrival one tile inward from the destination door so the door
  // is not immediately re-triggered on the next input.
  Coordinate dest = conn.destDoorPos;
  Coordinate arrivalTile = dest;
  if (dest.x == 0)
  {
    arrivalTile.x = 1;
  }
  else if (dest.x == Room::WIDTH - 1)
  {
    arrivalTile.x = Room::WIDTH - 2;
  }
  else if (dest.y == 0)
  {
    arrivalTile.y = 1;
  }
  else if (dest.y == Room::HEIGHT - 1)
  {
    arrivalTile.y = Room::HEIGHT - 2;
  }

  return arrivalTile;
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
