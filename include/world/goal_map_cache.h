#ifndef GOAL_MAP_CACHE_H
#define GOAL_MAP_CACHE_H

#include <cstddef>
#include <map>
#include <utility>

#include "core/coordinate.h"
#include "world/pathfinding.h"

struct Room;

/**
 * @brief Bounded LRU-lite cache of enemy Dijkstra goal maps.
 *
 * A goal map is a BFS distance grid rooted at a goal tile (see
 * computeGoalMap). Multiple enemies in the same room targeting the same
 * coordinate reuse a single cached map instead of recomputing per enemy.
 *
 * Cache entries remain valid as long as room geometry does not change. The
 * only invalidation the cache performs itself is a wholesale clear when
 * `kCap` is exceeded; callers can force this manually via clear().
 */
class GoalMapCache {
 public:
  /**
   * @brief Fetch (or lazily compute) the goal map rooted at `goal` for
   * `room`.
   *
   * Cache key is (room.roomID, goal). The returned reference is stable
   * until the next call that could trigger a cap-based clear, which is
   * currently only another `getOrCompute` call.
   *
   * @param room Room whose tile grid drives the BFS.
   * @param goal Target tile the map is rooted at (distance 0).
   * @return Const reference to the cached GoalMap.
   */
  const GoalMap& getOrCompute(const Room& room, Coordinate goal) const;

  /**
   * @brief Drop every cached goal map.
   *
   * Cheap; entries regenerate on the next `getOrCompute` call.
   */
  void clear() { cache_.clear(); }

  /**
   * @brief Number of goal maps currently cached (for tests / inspection).
   */
  std::size_t size() const { return cache_.size(); }

 private:
  /// Soft cap on cached goal maps. When exceeded, the cache is cleared
  /// wholesale — cheap since BFS is microseconds at Room::WIDTH x HEIGHT.
  /// Sized to comfortably hold the current player position plus a handful
  /// of active "last known" coords per room across several rooms.
  static constexpr std::size_t kCap = 32;

  /// Marked mutable so `getOrCompute` can stay const — the cache is an
  /// implementation detail, not observable state.
  mutable std::map<std::pair<int, Coordinate>, GoalMap> cache_;
};

#endif
