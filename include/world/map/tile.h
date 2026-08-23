// tile.h
#ifndef TILE_H
#define TILE_H

#include <cstdint>
#include <unordered_map>

#include "core/coordinate.h"

enum class TileType : std::uint8_t
{
  Wall,
  Floor,
  Door,
  Void,
  Pillar,
  EntryWay
};

struct TileAttributes
{
  char symbol;
  bool walkable;
};

class Tile
{
 public:
  /**
   * @brief Tile class represents a single tile on the map, with type, position,
   * and properties.
   *
   * @param type Type of tile. By default, TileType::Wall.
   * @param position Position of tile.
   */
  explicit Tile(TileType type = TileType::Wall,
                Coordinate position = Coordinate(0, 0));

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
  TileType getType() const { return type_; }

  /**
   * @brief Get the tile poisition.
   *
   * @return Coordinate
   */
  Coordinate getPosition() const { return position_; }

  /**
   * @brief Check if tile is currently within the viewer's field of view.
   *
   * @return bool
   */
  bool isVisible() const { return visible_; }

  /**
   * @brief Check if tile has been seen at least once. Persists once true,
   * except Void tiles are never marked explored.
   *
   * @return bool
   */
  bool isExplored() const { return explored_; }

  /**
   * @brief Mark this tile as currently visible, and — unless it is a Void
   * tile — permanently explored.
   */
  void reveal();

  /**
   * @brief Clear the currently-visible flag. Does not affect the explored
   * flag.
   */
  void clearVisible() { visible_ = false; }

  /**
   * @brief Convert symbol to tile type.
   *
   * @param ch The character for tile.
   *
   * @return TileType
   */
  static TileType charToTileType(char ch);

 private:
  TileType type_;
  Coordinate position_;
  bool visible_ = false;
  bool explored_ = false;

  // Static lookup table for tile attributes based on TileType.
  static const std::unordered_map<int, TileAttributes> typeAttributes_;
};

#endif