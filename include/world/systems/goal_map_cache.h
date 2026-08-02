#ifndef GOAL_MAP_CACHE_H
#define GOAL_MAP_CACHE_H

#include <cstddef>
#include <map>
#include <utility>

#include "core/coordinate.h"
#include "world/systems/pathfinding.h"

struct Room;

class GoalMapCache {
 public:
  /**
   * @brief Fetch (or lazily compute) the goal map rooted at `goal` for
   * `room`.
   *
   * @param room Room whose tile grid drives the BFS.
   * @param goal Target tile the map is rooted at (distance 0).
   * @return Const reference to the cached GoalMap.
   */
  const GoalMap& getOrCompute(const Room& room, Coordinate goal) const;

  /** @brief Drop every cached goal map. */
  void clear() { cache_.clear(); }

  /**
   * @brief Number of goal maps currently cached (for tests / inspection).
   */
  std::size_t size() const { return cache_.size(); }

 private:
  static constexpr std::size_t kCap = 32;
  mutable std::map<std::pair<int, Coordinate>, GoalMap> cache_;
};

#endif
