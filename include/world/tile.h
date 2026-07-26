// tile.h
#ifndef TILE_H
#define TILE_H

#include <unordered_map>

#include "core/coordinate.h"

enum class TileType { Wall, Floor, Door, Void, Pillar };

struct TileAttributes {
  char symbol;
  bool walkable;
};

class Tile {
 public:
  /**
   * @brief Tile class represents a single tile on the map, with type, position,
   * and properties.
   *
   * @param type Type of tile. By default, TileType::Wall.
   * @param position Position of tile.
   */
  Tile(TileType type = TileType::Wall, Coordinate position = Coordinate(0, 0));

  /**
   * @brief Get the tile symbol.
   *
   * @return char
   */
  char getSymbol() const;

  /**
   * @brief Check if tile is walkable.
   *
   * @return bool
   */
  bool isWalkable() const;

  /**
   * @brief Get the tile type.
   *
   * @return TileType
   */
  TileType getType() const { return type; }

  /**
   * @brief Get the tile poisition.
   *
   * @return Coordinate
   */
  Coordinate getPosition() const { return position; }

  /**
   * @brief Convert symbol to tile type.
   *
   * @param ch The character for tile.
   *
   * @return TileType
   */
  static TileType charToTileType(char ch);

 private:
  TileType type;
  Coordinate position;

  // Static lookup table for tile attributes based on TileType.
  static const std::unordered_map<int, TileAttributes> typeAttributes;
};

#endif