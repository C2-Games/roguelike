#include "world/systems/goal_map_cache.h"

#include <utility>

#include "objects/room/room.h"

const GoalMap& GoalMapCache::getOrCompute(const Room& room,
                                          Coordinate goal) const
{
  auto key = std::make_pair(room.roomID, goal);
  auto cachedEntry = cache_.find(key);
  if (cachedEntry != cache_.end())
  {
    return cachedEntry->second;
  }

  // prevent unbounded growth over long play sessions.
  if (cache_.size() >= kCap)
  {
    cache_.clear();
  }

  GoalMap map = computeGoalMap(room, goal);
  auto [inserted, _] = cache_.emplace(key, std::move(map));
  return inserted->second;
}
