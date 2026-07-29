#ifndef LEVEL_H
#define LEVEL_H

#include <memory>
#include <vector>

#include "core/coordinate.h"
#include "entities/enemy.h"
#include "entities/enemy_registry.h"
#include "world/goal_map_cache.h"
#include "world/pathfinding.h"
#include "world/room.h"
#include "world/room_graph.h"

struct GameServices;

/**
 * @brief Thin coordinator over RoomGraph, EnemyRegistry, and GoalMapCache.
 *
 * Ownership breakdown:
 *   - `roomGraph_` owns Rooms + the door graph + current-room cursor.
 *   - `enemyRegistry_` owns per-room enemy storage + first-visit flags.
 *   - `goalMapCache_` owns the enemy Dijkstra distance-grid cache.
 *
 * Public getters that used to live on Level (getRoomCount, getCurrentRoom,
 * getDoorConnection, etc.) are retained as thin delegations to roomGraph_
 * so downstream callers (layers, Game) don't need to rewire.
 */
class Level {
 public:
  /**
   * @brief Construct a Level and immediately build its room graph.
   *
   * @param roomCount Number of rooms to generate.
   * @param services  Shared services (RNG source). Consumed by RoomGraph
   *   for room selection and by EnemyRegistry for factory calls. Reference
   *   must outlive the Level.
   */
  explicit Level(int roomCount, GameServices& services);

  /**
   * @brief Load enemies for the starting room into the game's active list.
   *
   * @param activeEnemies Game's active enemy list to populate.
   */
  void loadInitialEnemies(std::vector<std::unique_ptr<Enemy>>& activeEnemies);

  /**
   * @brief Persist the current room's enemies and load the next room's.
   *
   * @param fromRoomID    Room being left.
   * @param toRoomID      Room being entered.
   * @param activeEnemies Game's active enemy list to swap in-place.
   */
  void transitionEnemies(int fromRoomID, int toRoomID,
                         std::vector<std::unique_ptr<Enemy>>& activeEnemies);

  /**
   * @brief Recompute FoV visibility for the current room.
   *
   * Clears the current room's visible grid, then marks every tile inside
   * player FoV as both visible and explored. Called once per frame from
   * Game::update() so a change to the player's sight radius takes effect on
   * the next render.
   *
   * @param origin World position of the FoV origin (the player).
   * @param fov Precomputed FoV mask defining which offsets are lit.
   */
  void updateVisibility(Coordinate origin, const FOV& fov);

  /**
   * @brief Fetch (and lazily compute) the enemy goal map for a given target.
   *
   * Delegates to an internal GoalMapCache. See GoalMapCache::getOrCompute
   * for the caching contract and reference-lifetime rules.
   *
   * @param roomID Room whose tile grid drives the BFS.
   * @param goal   Target tile the map is rooted at (distance 0).
   * @return Const reference to the cached GoalMap.
   */
  const GoalMap& getGoalMap(int roomID, Coordinate goal) const;

  /**
   * @brief Drop every cached goal map. Entries regenerate on the
   * next getGoalMap() call.
   */
  void clearGoalMapCache() { goalMapCache_.clear(); }

  // ---- Room-graph delegations ----
  int getRoomCount() const { return roomGraph_.getRoomCount(); }
  int getCurrentRoomID() const { return roomGraph_.getCurrentRoomID(); }
  const Room& getCurrentRoom() const { return roomGraph_.getCurrentRoom(); }
  void setCurrentRoomID(int id) { roomGraph_.setCurrentRoomID(id); }
  const DoorConnection* getDoorConnection(int roomID,
                                          Coordinate doorPos) const {
    return roomGraph_.getDoorConnection(roomID, doorPos);
  }

 private:
  GameServices& services;  ///< Injected RNG source (used by spawn logic).
  RoomGraph roomGraph_;    ///< Rooms + door graph + current-room cursor.
  EnemyRegistry
      enemyRegistry_;          ///< Per-room enemy storage + first-visit spawn.
  GoalMapCache goalMapCache_;  ///< Enemy pathfinding goal-map cache.
};

#endif
