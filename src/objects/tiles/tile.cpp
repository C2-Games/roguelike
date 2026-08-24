#include "objects/tiles/tile.h"

Tile::Tile(TileType type, Coordinate position)
    : type_(type), position_(position)
{}

// define all tile types and their attributes in one place.
const std::unordered_map<int, TileAttributes> Tile::typeAttributes_ = {
    {static_cast<int>(TileType::Wall), {'#', false}},
    {static_cast<int>(TileType::Floor), {'.', true}},
    {static_cast<int>(TileType::Door), {'+', true}},
    {static_cast<int>(TileType::Void), {' ', false}},
    {static_cast<int>(TileType::Pillar), {'o', false}},
    {static_cast<int>(TileType::EntryWay), {'.', true}},
};

char Tile::getSymbol() const
{
  return typeAttributes_.at(static_cast<int>(type_)).symbol;
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

TileType Tile::charToTileType(char ch)
{
  switch (ch)
  {
    case '#':
      return TileType::Wall;
    case '+':
      return TileType::Door;
    default:
      return TileType::Floor;
  }
}
