// tile.h
#ifndef TILE_H
#define TILE_H

#include <unordered_map>

#include "objects/coordinate.h"
#include "objects/tiles/tile_type.h"

struct TileAttributes
{
  char symbol;
  bool walkable;
};

struct Tile
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
   * @return True if the tile is currently visible.
   */
  bool isRevealed() const { return visible_; }

  /**
   * @brief Check if tile has been seen at least once. Persists once true,
   * except Void tiles are never marked explored.
   *
   * @return bool
   */
  bool isExplored() const { return explored_; }

  /**
   * @brief Set whether this tile is currently visible, and — unless it is a
   * Void tile — mark it permanently explored when becoming visible.
   *
   * @param visible True to reveal the tile, false to clear its visibility.
   */
  void toggleReveal(bool visible);

  /**
   * @brief Set whether this tile is currently occupied.
   *
   * @param occupied True if the tile is occupied, false otherwise.
   */
  void toggleOccupied(bool occupied) { occupied_ = occupied; }

  /**
   * @brief Check if tile is currently occupied.
   *
   * @return True if the tile is currently occupied.
   */
  bool isOccupied() const { return occupied_; }

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
  bool occupied_ = false;

  // static lookup table for tile attributes based on TileType.
  static const std::unordered_map<int, TileAttributes> typeAttributes_;
};

#endif
