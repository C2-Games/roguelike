#ifndef MAP_H
#define MAP_H

#include <map>

#include "objects/door_connections.h"

// door graph for a level: keyed by one side of a link, the mapped
// DoorConnection describes the room and door position on the other side.
using RoomConnections = std::map<DoorConnection, DoorConnection>;

#endif
