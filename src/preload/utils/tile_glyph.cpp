#include "preload/utils/tile_glyph.h"

namespace preload
{

wchar_t defaultGlyph(TileType type)
{
  switch (type)
  {
    case TileType::Floor:
    case TileType::EntryWay:
      return L'.';
    case TileType::Pillar:
      return L'o';
    case TileType::Door:
    case TileType::Void:
      return L' ';
    // wall and cap cells are always authored as box-drawing art by the room
    // loader, so no current path reaches these — plain last-resort values.
    case TileType::Wall:
      return L'#';
    case TileType::DoorCap:
      return L'+';
    case TileType::DoorLocked:
      return L'⚿';
  }
  return L' ';
}

}  // namespace preload
