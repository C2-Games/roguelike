#ifndef ROOM_H
#define ROOM_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "objects/coordinate.h"
#include "objects/entities/enemy.h"
#include "objects/room/room_types.h"
#include "objects/tiles/tile.h"

class Entity;
class Player;

struct Room
{
  static constexpr int WIDTH = 175;
  static constexpr int HEIGHT = 50;

  int roomID;
  std::string name;
  std::vector<std::vector<Tile>> tiles;

  // The doors map is keyed by the door's authored label (DoorNumber)
  // and contains the grid position of that door tile.
  std::map<DoorNumber, Coordinate> doors;

  std::vector<Coordinate> enemySpawns;
  std::vector<Coordinate> lootSpawns;
  std::vector<Coordinate> itemSpawns;

  bool enemiesSpawned = false;
  std::vector<std::unique_ptr<Enemy>> enemies;

  /**
   * @brief Construct an empty Room filled with default Wall tiles.
   *
   * @param id Unique room identifier.
   */
  explicit Room(int id);

  /**
   * @brief Look up the grid position of one of this room's doors.
   *
   * @param number The door's authored label from the room's .txt grid.
   * @return The door tile's coordinate.
   * @throws std::runtime_error if this room has no door with that label.
   */
  Coordinate doorAt(DoorNumber number) const;

  /**
   * @brief Compute the tile one step inward from a door position on the
   * room's boundary — the landing tile used both for door-transition
   * arrival and for marking EntryWay at load time. If the position isn't
   * on a room edge, it is returned unchanged.
   *
   * @param doorPos The door's grid position.
   * @return The tile one step inward from the door.
   */
  static Coordinate inwardOfDoor(Coordinate doorPos);

  /** @brief Reset every cell in the visible grid to false. */
  void clearVisible();

  /**
   * @brief Bounds-checked visibility query.
   *
   * @param x Column.
   * @param y Row.
   * @return bool True if the tile is inside the current FoV. False if out
   * of bounds.
   */
  bool isVisible(int x, int y) const;

  /**
   * @brief Bounds-checked explored query.
   *
   * @param x Column.
   * @param y Row.
   * @return bool True if the tile has been seen at least once. False if
   * out of bounds.
   */
  bool isExplored(int x, int y) const;

  /**
   * @brief Mark a tile as currently visible and permanently explored.
   *
   * @param x Column.
   * @param y Row.
   */
  void reveal(int x, int y);

  /**
   * @brief Find the live enemy standing on `pos`. Stays a Room member (unlike
   * the spawn/reap logic that lives outside Room) because Enemy and
   * Projectile, both game-object classes in their own right, call it
   * directly; moving it out from under Room would make those classes reach
   * upward into core/ instead of sideways into Room.
   *
   * @param pos Tile to test.
   * @param exclude Enemy skipped by pointer identity. Null considers every
   * enemy.
   * @return The enemy on `pos`, or nullptr when none is there.
   */
  Enemy* enemyAt(Coordinate pos, const Enemy* exclude = nullptr) const;

  /**
   * @brief Find the live entity (enemy or the player) standing on `pos`.
   *
   * @param pos Tile to test.
   * @param player The player to consider alongside this room's enemies.
   * @return The entity on `pos`, or nullptr when none is there.
   */
  Entity* entityAt(Coordinate pos, Player& player) const;
  const Entity* entityAt(Coordinate pos, const Player& player) const;

  Room(Room&&) = default;
  Room& operator=(Room&&) = default;

  Room(const Room&) = delete;
  Room& operator=(const Room&) = delete;
};

#endif