#ifndef PATHFINDING_H
#define PATHFINDING_H

#include <limits>
#include <vector>

#include "objects/coordinate.h"

struct Room;

// distance-to-goal grid produced by computeGoalMap.
using GoalMap = std::vector<std::vector<int>>;

// sentinel written into GoalMap cells the BFS never reaches.
inline constexpr int kUnreachable = std::numeric_limits<int>::max();

/**
 * @brief Build a Dijkstra / BFS goal map rooted at the given tile (player/last
 * seen position).
 *
 * @param room The room whose tile grid is used for wall lookups.
 * @param goal The tile the goal map is rooted at (distance 0).
 * @return Distance grid sized Room::WIDTH x Room::HEIGHT.
 */
GoalMap computeGoalMap(const Room& room, Coordinate goal);

#endif
