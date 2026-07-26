#include "world/pathfinding.h"

#include <queue>

#include "world/room.h"
#include "world/tile.h"

namespace {

// Returns true if the tile at (x, y) blocks 4-connected pathfinding for
// enemies. Walls, Void, Pillars, and Doors are all blocking; only Floor is
// passable. 
bool isBlocking(const Room& room, int x, int y) {
  if (x < 0 || x >= Room::WIDTH || y < 0 || y >= Room::HEIGHT) return true;
  TileType type = room.tiles[x][y].getType();
  return type != TileType::Floor;
}

}  // namespace

GoalMap computeGoalMap(const Room& room, Coordinate goal) {
  GoalMap map(Room::WIDTH,
              std::vector<int>(Room::HEIGHT, kUnreachable));

  // Reject out-of-bounds or blocking goal tiles up front. Returning the
  // all-kUnreachable map is a well-defined "no reachable path" result the
  // caller can treat uniformly.
  if (goal.x < 0 || goal.x >= Room::WIDTH || goal.y < 0 ||
      goal.y >= Room::HEIGHT) {
    return map;
  }
  if (isBlocking(room, goal.x, goal.y)) return map;

  std::queue<Coordinate> frontier;
  map[goal.x][goal.y] = 0;
  frontier.push(goal);

  // 4-connected neighbor offsets (N, E, S, W).
  static constexpr int kDx[4] = {0, 1, 0, -1};
  static constexpr int kDy[4] = {-1, 0, 1, 0};

  while (!frontier.empty()) {
    Coordinate current = frontier.front();
    frontier.pop();
    int nextDist = map[current.x][current.y] + 1;

    // Explore 4-connected neighbors of the current tile.
    for (int i = 0; i < 4; ++i) {
      int nx = current.x + kDx[i];
      int ny = current.y + kDy[i];
      if (isBlocking(room, nx, ny)) continue;
      if (map[nx][ny] != kUnreachable) continue;  // already visited
      map[nx][ny] = nextDist;
      // Push the neighbor onto the frontier for further exploration.
      frontier.push({nx, ny});
    }
  }

  return map;
}
