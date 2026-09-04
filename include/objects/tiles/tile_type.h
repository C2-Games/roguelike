#ifndef TILE_TYPE_H
#define TILE_TYPE_H

#include <cstdint>

enum class TileType : std::uint8_t
{
  Wall,
  Floor,
  Door,
  Void,
  Pillar,
  EntryWay,
  DoorCap,
  DoorLocked
};

#endif
