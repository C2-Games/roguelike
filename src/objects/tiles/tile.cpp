#include "objects/tiles/tile.h"

Tile::Tile(TileType type, Coordinate position)
    : type_(type), position_(position)
{}

wchar_t Tile::getSymbol() const { return symbol_; }

bool Tile::isWalkable() const
{
  switch (type_)
  {
    case TileType::Floor:
    case TileType::Door:
    case TileType::EntryWay:
      return true;
    case TileType::Wall:
    case TileType::Void:
    case TileType::Pillar:
    case TileType::DoorCap:
    case TileType::DoorLocked:
      return false;
  }
  return false;
}

void Tile::toggleReveal(bool visible)
{
  visible_ = visible;
  // only mark explored if the tile is not Void.
  if (visible_ && type_ != TileType::Void)
  {
    explored_ = true;
  }
}
