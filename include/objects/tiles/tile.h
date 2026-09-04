// tile.h
#ifndef TILE_H
#define TILE_H

#include "objects/coordinate.h"
#include "objects/tiles/tile_type.h"

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
   * @brief Set the tile's display symbol.
   *
   * @param symbol The wide-char glyph to display.
   */
  void setSymbol(wchar_t symbol) { symbol_ = symbol; }

  /**
   * @brief Get the tile's display symbol.
   *
   * @return The wide-char glyph to render for this tile.
   */
  wchar_t getSymbol() const;

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

 private:
  TileType type_;
  Coordinate position_;
  // the room loader sets this for every tile; there is no type-based fallback.
  wchar_t symbol_ = L' ';
  bool visible_ = false;
  bool explored_ = false;
  bool occupied_ = false;
};

#endif
