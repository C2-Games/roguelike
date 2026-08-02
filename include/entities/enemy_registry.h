#ifndef ENEMY_REGISTRY_H
#define ENEMY_REGISTRY_H

#include <map>
#include <memory>
#include <vector>

class Enemy;
struct Room;
struct GameServices;

class EnemyRegistry {
 public:
  /**
   * @brief Construct with shared services (RNG for factory calls).
   *
   * @param services Reference stored for the registry's lifetime; must
   *                 outlive the EnemyRegistry.
   */
  explicit EnemyRegistry(GameServices& services);

  /**
   * @brief Move `roomID`'s enemies into `active`. On first visit for
   * `roomID`, factory-rolls the initial set first.
   *
   * @param roomID Room being entered.
   * @param room   Room object providing the tile grid for spawn placement.
   * @param active Game's active-enemy list; populated in-place.
   */
  void loadForRoom(int roomID, const Room& room,
                   std::vector<std::unique_ptr<Enemy>>& active);

  /**
   * @brief Park `active` into storage for `fromRoomID`, then load
   * `toRoomID`. Full enemy state (HP, position) is preserved across
   * transitions.
   *
   * @param fromRoomID Room being left.
   * @param toRoomID   Room being entered.
   * @param toRoom     Destination room, for factory spawn placement on
   *                   first visit.
   * @param active     Game's active-enemy list; swapped in-place.
   */
  void transitionActive(int fromRoomID, int toRoomID, const Room& toRoom,
                        std::vector<std::unique_ptr<Enemy>>& active);

  /** @brief Drop dead enemies from `active`. */
  static void reap(std::vector<std::unique_ptr<Enemy>>& active);

 private:
  GameServices& services_;
  std::map<int, std::vector<std::unique_ptr<Enemy>>> perRoom_;
  std::map<int, bool> visited_;

  /**
   * @brief If `roomID` has never been visited, factory-roll its enemies
   * into `perRoom_` and mark visited. No-op on subsequent calls.
   */
  void ensureSpawned(int roomID, const Room& room);
};

#endif
