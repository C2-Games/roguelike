#ifndef ROOM_TYPES_H
#define ROOM_TYPES_H

#include <array>
#include <string>

// door number based on the order of doors in the room's template file.
using DoorNumber = int;

// one enemy spawn-table entry from a room's JSON metadata.
struct EnemySpawnConfig
{
  std::string name;
  int tier;
  std::array<int, 2> range;
};

#endif
