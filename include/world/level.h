#ifndef LEVEL_H
#define LEVEL_H

#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "core/coordinate.h"
#include "entities/enemy.h"
#include "world/pathfinding.h"
#include "world/room.h"

/**
 * @brief Describes a one-way portal from a door tile to a destination room.
 *
 * Two DoorConnection records (one per direction) form a bidirectional link
 * between rooms. Stored in Level::doorConnections keyed by (roomID, doorPos).
 */
struct DoorConnection {
  int destRoomID;          ///< ID of the room the door leads to.
  Coordinate destDoorPos;  ///< Position of the matching door in that room.
};

/**
 * @brief Represents the game map, including all rooms and their connections.
 *
 * Level owns every Room, manages door-to-door links, and maintains per-room
 * enemy lists that persist across visits to support future loot/HP retention.
 */
class Level {
 public:
  /**
   * @brief Construct a Level and immediately generate its rooms.
   *
   * @param roomCount Number of rooms to generate.
   */
  Level(int roomCount);

  /**
   * @brief Add a room to the level.
   *
   * @param room Room to add (moved in).
   */
  void addRoom(Room room);

  /**
   * @brief Procedurally generate all rooms and wire their door connections.
   *
   * Rooms are created with randomly chosen RoomShapes and linked in a chain
   * so every room is reachable from the starting room.
   */
  void generate();

  /**
   * @brief Look up the door connection for a given room and door position.
   *
   * @param roomID  The room the player is currently in.
   * @param doorPos The tile coordinate of the door being stepped on.
   * @return Pointer to the DoorConnection, or nullptr if the door is unlinked.
   */
  const DoorConnection* getDoorConnection(int roomID, Coordinate doorPos) const;

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
   * player FoV as both visible and explored. Called
   * once per frame from Game::update() so a change to the player's sight
   * radius takes effect on the next render.
   *
   * @param origin World position of the FoV origin (the player).
   * @param fov Precomputed FoV mask defining which offsets are lit.
   */
  void updateVisibility(Coordinate origin, const FOV& fov);

  /**
   * @brief Transition to a different room by ID.
   *
   * @param id Target room ID.
   */
  void setCurrentRoomID(int id) { currentRoomID = id; }

  /**
   * @brief Fetch (and lazily compute) the enemy goal map for a given target.
   *
   * A goal map is a BFS distance grid rooted at `goal`, treating Walls, Void,
   * Pillars, and Doors as blocking (see computeGoalMap). Multiple enemies in
   * the same room targeting the same coordinate reuse a single cached map.
   *
   * The cache is keyed by (roomID, goal) and remains valid as long as room
   * geometry does not change. When the cache grows beyond a soft cap it is
   * cleared wholesale; entries are then recomputed on demand.
   *
   * Callers must not retain the returned reference across a call to
   * clearGoalMapCache() (nor across a call that could trigger the internal
   * cap-based eviction — currently only another getGoalMap() call).
   *
   * @param roomID Room whose tile grid drives the BFS.
   * @param goal   Target tile the map is rooted at (distance 0).
   * @return Const reference to the cached GoalMap.
   */
  const GoalMap& getGoalMap(int roomID, Coordinate goal);

  /**
   * @brief Drop every cached goal map.
   *
   * Cheap; entries regenerate on next getGoalMap() call. Intended for use if
   * room geometry ever mutates (not currently possible)
   */
  void clearGoalMapCache() { goalMapCache.clear(); }

  // Getters
  int getRoomCount() const { return roomCount; }
  int getCurrentRoomID() const { return currentRoomID; }
  const Room& getCurrentRoom() const { return roomList.at(currentRoomID); }

 private:
  int roomCount;
  int currentRoomID = 0;
  std::map<int, Room> roomList;                   ///< All rooms keyed by ID.
  std::vector<std::vector<int>> roomConnections;  ///< Adjacency list.
  std::map<std::pair<int, Coordinate>, DoorConnection>
      doorConnections;  ///< (roomID, doorPos) → dest.
  std::map<int, std::vector<std::unique_ptr<Enemy>>>
      roomEnemies;                  ///< Per-room enemy lists.
  std::map<int, bool> roomVisited;  ///< Lazy-init visit flags.

  /// Lazily-populated cache of goal maps keyed by (roomID, target coord).
  /// Since room geometry is immutable at runtime, entries stay valid until
  /// evicted for capacity reasons. See getGoalMap() for reference-lifetime
  /// rules.
  std::map<std::pair<int, Coordinate>, GoalMap> goalMapCache;

  /// Soft cap on cached goal maps. When exceeded, the cache is cleared
  /// wholesale — cheap since BFS is microseconds at Room::WIDTH x HEIGHT.
  /// Sized to comfortably hold the current player position plus a handful
  /// of active "last known" coords per room across several rooms.
  static constexpr std::size_t kGoalMapCacheCap = 32;

  /**
   * @brief Spawn fresh enemies for the given room and store them in
   * roomEnemies.
   *
   * @param roomID Room to populate.
   */
  void spawnEnemiesForRoom(int roomID);
};

#endif
