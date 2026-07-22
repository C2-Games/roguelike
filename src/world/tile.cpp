#include "world/tile.h"

Tile::Tile(TileType type, Coordinate position)
    : type(type), position(position) {}

// Define all tile types and their attributes in ONE place
const std::unordered_map<int, TileAttributes> Tile::typeAttributes = {
    {static_cast<int>(TileType::Wall), {'#', false}},
    {static_cast<int>(TileType::Floor), {'.', true}},
    {static_cast<int>(TileType::Door), {'+', true}},
    {static_cast<int>(TileType::Void), {' ', false}},
    {static_cast<int>(TileType::Pillar), {'o', false}},
};

char Tile::getSymbol() const {
  return typeAttributes.at(static_cast<int>(type)).symbol;
}

bool Tile::isWalkable() const {
  return typeAttributes.at(static_cast<int>(type)).walkable;
}

TileType Tile::charToTileType(char ch) {
  switch (ch) {
    case '#':
      return TileType::Wall;
    case '+':
      return TileType::Door;
    default:
      return TileType::Floor;
  }
}
