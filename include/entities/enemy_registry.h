#ifndef ENEMY_REGISTRY_H
#define ENEMY_REGISTRY_H

#include <map>
#include <memory>
#include <vector>

class Enemy;
struct Room;
struct GameServices;

/**
 * @brief Per-room enemy storage and first-visit spawn orchestration.
 *
 * Rooms retain their enemies across visits — leaving a room and returning
 * finds the same enemies with the same HP and positions. First-visit
 * detection triggers a factory roll via `enemy_factory::rollForRoom`.
 *
 * Storage is keyed by roomID. Private member names (`perRoom_`, `visited_`)
 * are deliberately generic so this class serves as a template for future
 * per-room stores
 */
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

  /**
   * @brief Drop dead enemies from `active`.
   *
   * Closes the EntityLayer TODO about cleaning up dead objects. Not
   * currently called anywhere; callers can invoke it once per frame or per
   * turn as their combat model matures.
   */
  static void reap(std::vector<std::unique_ptr<Enemy>>& active);

 private:
  GameServices& services_;
  std::map<int, std::vector<std::unique_ptr<Enemy>>>
      perRoom_;                  ///< Enemy vectors keyed by roomID.
  std::map<int, bool> visited_;  ///< First-visit flags keyed by roomID.

  /**
   * @brief If `roomID` has never been visited, factory-roll its enemies
   * into `perRoom_` and mark visited. No-op on subsequent calls.
   */
  void ensureSpawned(int roomID, const Room& room);
};

#endif
