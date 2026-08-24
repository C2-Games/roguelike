#include "systems/movement/pathfinding.h"

#include <queue>

#include "objects/room/room.h"
#include "objects/tiles/tile_type.h"

namespace
{

bool isBlocking(const Room& room, Coordinate pos)
{
  return room.getTileType(pos) != TileType::Floor;
}

}  // namespace

GoalMap computeGoalMap(const Room& room, Coordinate goal)
{
  GoalMap map(Room::WIDTH, std::vector<int>(Room::HEIGHT, UNREACHABLE));

  // reject out-of-bounds or blocking goal tiles up front. returning the
  // all-UNREACHABLE map is a well-defined "no reachable path" result the
  // caller can treat uniformly.
  if (isBlocking(room, goal))
  {
    return map;
  }

  std::queue<Coordinate> frontier;
  map[goal.x][goal.y] = 0;
  frontier.push(goal);

  // 4-connected neighbor offsets (N, E, S, W).
  static constexpr int DX[4] = {0, 1, 0, -1};
  static constexpr int DY[4] = {-1, 0, 1, 0};

  while (!frontier.empty())
  {
    Coordinate current = frontier.front();
    frontier.pop();
    int nextDist = map[current.x][current.y] + 1;

    // explore 4-connected neighbors of the current tile.
    for (int i = 0; i < 4; ++i)
    {
      Coordinate neighbor(current.x + DX[i], current.y + DY[i]);
      if (isBlocking(room, neighbor))
      {
        continue;
      }
      if (map[neighbor.x][neighbor.y] != UNREACHABLE)
      {
        continue;  // already visited.
      }
      map[neighbor.x][neighbor.y] = nextDist;
      // push the neighbor onto the frontier for further exploration.
      frontier.push(neighbor);
    }
  }

  return map;
}
