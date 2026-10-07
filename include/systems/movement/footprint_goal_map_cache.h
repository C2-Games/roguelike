#ifndef FOOTPRINT_GOAL_MAP_CACHE_H
#define FOOTPRINT_GOAL_MAP_CACHE_H

#include <cstddef>
#include <map>
#include <tuple>

#include "objects/coordinate.h"
#include "objects/entities/entity_symbol.h"
#include "systems/movement/footprint_pathfinding.h"

struct Room;

class FootprintGoalMapCache
{
 public:
  /**
   * @brief Fetch (or lazily compute) the footprint goal map rooted at
   * `goal` for `room` and `baseSymbol`.
   *
   * @param room Room whose tile grid drives the BFS.
   * @param goal Target tile footprints should end up adjacent to.
   * @param baseSymbol The entity's always-Horizontal-authored symbol grid.
   * @return Const reference to the cached FootprintGoalMap.
   */
  const FootprintGoalMap& getOrCompute(const Room& room, Coordinate goal,
                                       const EntitySymbol& baseSymbol) const;

  /** @brief Drop every cached footprint goal map. */
  void clear() { cache_.clear(); }

  /**
   * @brief Number of footprint goal maps currently cached.
   *
   * @return Count of currently cached footprint goal maps.
   */
  std::size_t size() const { return cache_.size(); }

 private:
  static constexpr std::size_t CAP = 16;
  mutable std::map<std::tuple<int, Coordinate, EntitySymbol>, FootprintGoalMap>
      cache_;
};

#endif
