#ifndef DOOR_CONNECTIONS_H
#define DOOR_CONNECTIONS_H

#include "objects/coordinate.h"

// one door's link to the door on the other side, in another room.
struct DoorConnection
{
  int roomID = 0;
  Coordinate doorPosition;

  bool operator<(const DoorConnection& other) const
  {
    if (roomID != other.roomID)
    {
      return roomID < other.roomID;
    }
    return doorPosition < other.doorPosition;
  }
};

#endif
