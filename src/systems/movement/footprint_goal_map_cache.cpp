#include "systems/movement/footprint_goal_map_cache.h"

#include <tuple>
#include <utility>

#include "objects/room/room.h"

const FootprintGoalMap& FootprintGoalMapCache::getOrCompute(
    const Room& room, Coordinate goal, const EntitySymbol& baseSymbol) const
{
  auto key = std::make_tuple(room.getRoomID(), goal, baseSymbol);
  auto cachedEntry = cache_.find(key);
  if (cachedEntry != cache_.end())
  {
    return cachedEntry->second;
  }

  // prevent unbounded growth over long play sessions.
  if (cache_.size() >= CAP)
  {
    cache_.clear();
  }

  FootprintGoalMap map = computeFootprintGoalMap(room, goal, baseSymbol);
  auto [inserted, _] = cache_.emplace(key, std::move(map));
  return inserted->second;
}
