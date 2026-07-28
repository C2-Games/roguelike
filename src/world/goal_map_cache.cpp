#include "world/goal_map_cache.h"

#include <utility>

#include "world/room.h"

const GoalMap& GoalMapCache::getOrCompute(const Room& room,
                                          Coordinate goal) const {
  auto key = std::make_pair(room.roomID, goal);
  auto it = cache_.find(key);
  if (it != cache_.end()) return it->second;

  // Prevent unbounded growth over long play sessions. Wholesale clear is
  // intentional: entries regenerate cheaply on demand and the cache warms
  // back up within a handful of enemy turns.
  if (cache_.size() >= kCap) cache_.clear();

  GoalMap map = computeGoalMap(room, goal);
  auto [inserted, _] = cache_.emplace(key, std::move(map));
  return inserted->second;
}
