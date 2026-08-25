#ifndef ROOM_H
#define ROOM_H

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "objects/coordinate.h"
#include "objects/room/room_dimensions.h"
#include "objects/tiles/tile.h"
#include "objects/tiles/tile_type.h"

// door number based on the order of doors in the room's template file.
using DoorNumber = int;

struct Room
{
  static constexpr int WIDTH = ROOM_WIDTH;
  static constexpr int HEIGHT = ROOM_HEIGHT;

  /**
   * @brief Construct an empty Room filled with default Wall tiles.
   *
   * @param id Unique room identifier.
   */
  explicit Room(int id);

  /**
   * @brief Get the room's unique identifier.
   *
   * @return The room ID.
   */
  int getRoomID() const { return roomID_; }

  /**
   * @brief Get the room's display name.
   *
   * @return The room name.
   */
  const std::string& getName() const { return name_; }

  /**
   * @brief Set the room's display name.
   *
   * @param name The new room name.
   */
  void setName(std::string name) { name_ = std::move(name); }

  /**
   * @brief Get every door this room has, keyed by authored label.
   *
   * @return The door number to grid position map.
   */
  const std::map<DoorNumber, Coordinate>& getDoors() const { return doors_; }

  /**
   * @brief Register a door at a grid position.
   *
   * @param number The door's authored label from the room's .txt grid.
   * @param pos The door tile's grid position.
   */
  void addDoor(DoorNumber number, Coordinate pos)
  {
    doors_.emplace(number, pos);
  }

  /**
   * @brief Bounds-checked tile write. No-op if `pos` is outside the room.
   *
   * @param pos Grid position to write.
   * @param tile Tile to place at `pos`.
   */
  void setTile(Coordinate pos, Tile tile);

  /**
   * @brief Bounds-checked visibility query.
   *
   * @param pos Grid position to test.
   * @return True if the tile is inside the current FoV. False if out of
   * bounds.
   */
  bool isVisible(Coordinate pos) const;

  /**
   * @brief Bounds-checked explored query.
   *
   * @param pos Grid position to test.
   * @return True if the tile has been seen at least once. False if out of
   * bounds.
   */
  bool isExplored(Coordinate pos) const;

  /**
   * @brief Set whether a tile is currently visible. No-op if `pos` is
   * outside the room.
   *
   * @param pos Grid position to update.
   * @param visible True to reveal the tile, false to clear its visibility.
   */
  void toggleReveal(Coordinate pos, bool visible);

  /**
   * @brief Bounds-checked occupied query.
   *
   * @param pos Grid position to test.
   * @return True if the tile is occupied. False if out of bounds.
   */
  bool isOccupied(Coordinate pos) const;

  /**
   * @brief Set whether a tile is currently occupied. No-op if `pos` is
   * outside the room.
   *
   * @param pos Grid position to update.
   * @param occupied True if the tile is occupied, false otherwise.
   */
  void toggleOccupied(Coordinate pos, bool occupied);

  /**
   * @brief Bounds-checked walkable query.
   *
   * @param pos Grid position to test.
   * @return True if the tile is walkable. False if out of bounds.
   */
  bool isWalkable(Coordinate pos) const;

  /**
   * @brief Bounds-checked tile type query.
   *
   * @param pos Grid position to test.
   * @return The tile's type, or TileType::Wall if out of bounds.
   */
  TileType getTileType(Coordinate pos) const;

  /**
   * @brief Bounds-checked tile symbol query.
   *
   * @param pos Grid position to test.
   * @return The tile's display symbol, or ' ' if out of bounds.
   */
  char getTileSymbol(Coordinate pos) const;

  /** @brief Reset every cell in the visible grid to false. */
  void clearVisible();

  /**
   * @brief Check whether a grid position lies within the room's bounds.
   *
   * @param position Grid position to test.
   * @return True if the position is inside [0, WIDTH) x [0, HEIGHT).
   */
  static bool inBounds(Coordinate position)
  {
    return position.x >= 0 && position.x < WIDTH && position.y >= 0 &&
           position.y < HEIGHT;
  }

  Room(Room&&) = default;
  Room& operator=(Room&&) = default;

  Room(const Room&) = delete;
  Room& operator=(const Room&) = delete;

 private:
  int roomID_;
  std::string name_;
  std::vector<std::vector<Tile>> tiles_;

  // the doors map is keyed by the door's authored label (DoorNumber)
  // and contains the grid position of that door tile.
  std::map<DoorNumber, Coordinate> doors_;
};

#endif
