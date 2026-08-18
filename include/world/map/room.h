#ifndef ROOM_H
#define ROOM_H

#include <filesystem>
#include <string>
#include <vector>

#include "core/coordinate.h"
#include "entities/room_enemy_state.h"
#include "world/map/level_config.h"
#include "world/map/tile.h"

class EnemyCatalog;

struct Room
{
  static constexpr int WIDTH = 175;
  static constexpr int HEIGHT = 50;

  int roomID;
  std::string name;
  std::vector<std::vector<Tile>> tiles;
  std::vector<Coordinate> doorPositions;

  std::vector<Coordinate> enemySpawns;
  std::vector<Coordinate> lootSpawns;
  std::vector<Coordinate> itemSpawns;

  RoomEnemyState enemyState;

  /**
   * @brief Construct an empty Room filled with default Wall tiles.
   *
   * @param id Unique room identifier.
   */
  explicit Room(int id);

  /**
   * @brief Load a Room from a text file authored under assets/rooms/.
   *
   * @param roomID Unique identifier assigned to the loaded room.
   * @param path   Path to the room file.
   * @return A fully populated Room ready to be added to a RoomGraph.
   */
  static Room loadFromFile(int roomID, const std::filesystem::path& path);

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

  std::vector<std::unique_ptr<Enemy>>& enemies()
  {
    return enemyState.enemies();
  }
  const std::vector<std::unique_ptr<Enemy>>& enemies() const
  {
    return enemyState.enemies();
  }

  /**
   * @brief Roll this room's enemies on first call; a no-op after.
   *
   * @param spawnTable The room's authored enemy entries.
   * @param catalog    Resolves each entry's type/tier to stats.
   * @param services   RNG source for the factory roll.
   */
  void ensureEnemiesSpawned(const std::vector<EnemySpawnConfig>& spawnTable,
                            const EnemyCatalog& catalog, GameServices& services)
  {
    enemyState.ensureSpawned(*this, spawnTable, catalog, services);
  }

  Room(Room&&) = default;
  Room& operator=(Room&&) = default;

  Room(const Room&) = delete;
  Room& operator=(const Room&) = delete;
};

#endif