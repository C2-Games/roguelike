#ifndef ROOM_H
#define ROOM_H

#include <map>
#include <string>
#include <vector>

#include "core/coordinate.h"
#include "entities/room_enemy_state.h"
#include "world/map/room_types.h"
#include "world/map/tile.h"

class EnemyCatalog;
class Entity;
class FOV;
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

  RoomEnemyState enemyState;

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
   * @brief Recompute this room's per-tile visibility from a viewer position.
   *
   * @param origin World position of the viewer (typically the player).
   * @param fov Precomputed FoV mask defining which offsets are lit.
   */
  void updateVisibility(Coordinate origin, const FOV& fov);

  /**
   * @brief Update visibility for a step, falling back to a full recompute.
   *
   * @param previousOrigin Viewer position before the step.
   * @param origin Viewer position after the step.
   * @param fov FoV mask, unchanged since the previous frame.
   */
  void updateVisibility(Coordinate previousOrigin, Coordinate origin,
                        const FOV& fov);

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
   * @brief Find the live enemy standing on `pos`.
   *
   * @param pos Tile to test.
   * @param exclude Enemy skipped by pointer identity. Null considers every
   * enemy.
   * @return The enemy on `pos`, or nullptr when none is there.
   */
  Enemy* enemyAt(Coordinate pos, const Enemy* exclude = nullptr) const
  {
    return enemyState.enemyAt(pos, exclude);
  }

  /**
   * @brief Find the live entity (enemy or the player) standing on `pos`.
   *
   * @param pos Tile to test.
   * @param player The player to consider alongside this room's enemies.
   * @return The entity on `pos`, or nullptr when none is there.
   */
  Entity* entityAt(Coordinate pos, Player& player) const;
  const Entity* entityAt(Coordinate pos, const Player& player) const;

  /**
   * @brief Roll this room's enemies on first call; a no-op after.
   *
   * @param spawnTable The room's authored enemy entries.
   * @param catalog    Resolves each entry's name/tier to stats.
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

 private:
  /**
   * @brief Incrementally update visibility for a single unit-cardinal step.
   *
   * @param previousOrigin Viewer position before the step.
   * @param origin Viewer position after the step.
   * @param fov FoV mask, unchanged since the previous frame.
   * @return bool True if applied; false if the step wasn't a precomputed unit
   * cardinal move, meaning the caller must fall back to updateVisibility().
   */
  bool updateVisibilityDelta(Coordinate previousOrigin, Coordinate origin,
                             const FOV& fov);
};

#endif