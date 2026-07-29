#ifndef LEVEL_H
#define LEVEL_H

#include <map>
#include <memory>
#include <vector>

#include "core/coordinate.h"
#include "entities/enemy.h"
#include "world/goal_map_cache.h"
#include "world/pathfinding.h"
#include "world/room.h"
#include "world/room_graph.h"

struct GameServices;

/**
 * @brief Coordinates the room map, per-room enemy lifecycle, visibility, and
 * enemy pathfinding cache.
 *
 * Ownership breakdown:
 *   - `roomGraph_` owns Rooms + the door graph + current-room cursor.
 *   - `goalMapCache_` owns the enemy Dijkstra distance-grid cache.
 *   - Level owns per-room enemy vectors and the first-visit flags that drive
 *     enemy persistence across room re-entries.
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
   * @param services  Shared services (RNG source) used by room selection
   *   and enemy spawn placement. Stored by reference; must outlive the
   *   Level.
   */
  explicit Level(int roomCount, GameServices& services);

  /**
   * @brief Load enemies for the starting room into the game's active list.
   *
   * Called once at game start. Generates enemies for room 0 and moves them
   * into activeEnemies.
   *
   * @param activeEnemies Game's active enemy list to populate.
   */
  void loadInitialEnemies(std::vector<std::unique_ptr<Enemy>>& activeEnemies);

  /**
   * @brief Persist the current room's enemies and load the next room's.
   *
   * Moves activeEnemies into storage for fromRoomID, then moves stored
   * enemies for toRoomID into activeEnemies (spawning fresh ones on first
   * visit). Full enemy state — health, position — is preserved across
   * transitions, enabling future loot/HP persistence.
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
  GameServices& services;      ///< Injected RNG source (used by spawn logic).
  RoomGraph roomGraph_;        ///< Rooms + door graph + current-room cursor.
  GoalMapCache goalMapCache_;  ///< Enemy pathfinding goal-map cache.
  std::map<int, std::vector<std::unique_ptr<Enemy>>>
      roomEnemies;                  ///< Per-room enemy lists.
  std::map<int, bool> roomVisited;  ///< Lazy-init first-visit flags.

  /**
   * @brief Spawn fresh enemies for the given room and store them in
   * roomEnemies.
   *
   * @param roomID Room to populate.
   */
  void spawnEnemiesForRoom(int roomID);
};

#endif
