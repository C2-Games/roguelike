#ifndef PATHFINDING_H
#define PATHFINDING_H

#include <limits>
#include <vector>

#include "core/coordinate.h"

// Forward declaration for Room to avoid circular dependency. Because we only
// need a reference to Room in the function signature, we don't need the full
// definition here.
struct Room;

/**
 * @brief Distance-to-goal grid produced by computeGoalMap.
 *
 * Indexed [x][y] to match Room::tiles. Each cell holds the shortest
 * 4-connected step distance from that tile back to the goal, treating Walls,
 * Void, Pillars, and Doors as blocking. Type Alias for a 2-D grid of ints.
 */
using GoalMap = std::vector<std::vector<int>>;

/**
 * @brief Sentinel written into GoalMap cells the BFS never reaches.
 *
 * Chosen to be the maximum representable int so a plain `<` comparison in the
 * enemy neighbor-selection loop naturally ranks unreachable tiles lower than
 * any reachable tile.
 */
inline constexpr int kUnreachable = std::numeric_limits<int>::max();

/**
 * @brief Build a Dijkstra / BFS goal map rooted at the given tile (player/last
 * seen position).
 *
 * Performs a single 4-connected BFS from `goal` outward across the room grid.
 * Blocking tiles: Wall, Void, Pillar, Door. All other tiles are passable at
 * uniform cost 1.
 *
 * If `goal` itself is a blocking tile or falls outside the room bounds, the
 * returned map is entirely kUnreachable.
 *
 * @param room The room whose tile grid is used for wall lookups.
 * @param goal The tile the goal map is rooted at (distance 0).
 * @return GoalMap Distance grid sized Room::WIDTH x Room::HEIGHT.
 */
GoalMap computeGoalMap(const Room& room, Coordinate goal);

#endif
