#include "objects/tiles/tile.h"

Tile::Tile(TileType type, Coordinate position)
    : type_(type), position_(position)
{}

// define all tile types and their attributes in one place.
const std::unordered_map<int, TileAttributes> Tile::typeAttributes_ = {
    {static_cast<int>(TileType::Wall), {L'#', false}},
    {static_cast<int>(TileType::Floor), {L'.', true}},
    {static_cast<int>(TileType::Door), {L' ', true}},
    {static_cast<int>(TileType::Void), {L' ', false}},
    {static_cast<int>(TileType::Pillar), {L'o', false}},
    {static_cast<int>(TileType::EntryWay), {L'.', true}},
    {static_cast<int>(TileType::DoorCap), {L'+', false}},
    {static_cast<int>(TileType::DoorLocked), {L'⚿', false}},
};

wchar_t Tile::getSymbol() const
{
  return symbol_ != 0 ? symbol_
                      : typeAttributes_.at(static_cast<int>(type_)).symbol;
}

bool Tile::isWalkable() const
{
  return typeAttributes_.at(static_cast<int>(type_)).walkable;
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
