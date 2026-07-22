#ifndef ROOM_H
#define ROOM_H

#include <filesystem>
#include <string>
#include <vector>

#include "core/coordinate.h"
#include "world/tile.h"

/**
 * @brief Represents a single room on the game map.
 *
 * A Room owns a fixed-size 2-D tile grid (WIDTH × HEIGHT). Tiles outside the
 * shaped floor area are TileType::Void. Door positions on the room's perimeter
 * walls are tracked in doorPositions so Level can wire room connections.
 *
 * Rooms are authored as text files under assets/rooms/ and loaded via
 * Room::loadFromFile.
 */
struct Room {
  static constexpr int WIDTH = 175;  ///< Tile columns per room.
  static constexpr int HEIGHT = 50;  ///< Tile rows per room.

  int roomID;                             ///< Unique identifier for this room.
  std::string name;                       ///< Human-readable name from @name.
  std::vector<std::vector<Tile>> tiles;   ///< Tile grid, indexed [x][y].
  std::vector<Coordinate> doorPositions;  ///< Door positions in A,B,C,... order.

  /// True when the tile is inside the player's current FoV. Recomputed each
  /// frame by Level::updateVisibility. Indexed [x][y].
  std::vector<std::vector<bool>> visible;

  /// True once the tile has ever been visible. Persists across frames and
  /// room re-entries so the map is "remembered" once seen. Indexed [x][y].
  std::vector<std::vector<bool>> explored;

  /**
   * @brief Construct an empty Room filled with default Wall tiles.
   *
   * @param id Unique room identifier.
   */
  Room(int id);

  /**
   * @brief Load a Room from a text file authored under assets/rooms/.
   *
   * File format:
   *   - Optional header lines starting with `@key: value` (e.g. `@name: foo`,
   *     `@levels: 1-3`). Unrecognized keys are ignored.
   *   - Grid begins at the first non-`@` line. Must be exactly HEIGHT rows,
   *     each exactly WIDTH characters wide.
   *   - Tile legend: `#` Wall, `.` Floor, `o` Pillar, ` ` Void.
   *   - Doors are labelled A-Z (or a-z). Each labelled cell becomes a Door
   *     tile; doorPositions is populated in alphabetical order so the file
   *     author controls which door is index 0, 1, etc.
   *
   * Throws std::runtime_error on any parse or dimension violation.
   *
   * @param roomID Unique identifier assigned to the loaded room.
   * @param path   Path to the room file.
   * @return A fully populated Room ready to be added to a Level.
   */
  static Room loadFromFile(int roomID, const std::filesystem::path& path);

  /**
   * @brief Reset every cell in the visible grid to false.
   *
   * Called at the start of each visibility pass before the new FoV cells
   * are marked visible. The explored grid is left untouched.
   */
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
   * No-op if (x, y) falls outside the room grid so callers may pass raw
   * FoV coordinates without clamping first.
   *
   * @param x Column.
   * @param y Row.
   */
  void reveal(int x, int y);

  Room(Room&&) = default;
  Room& operator=(Room&&) = default;

  Room(const Room&) = delete;
  Room& operator=(const Room&) = delete;
};

#endif