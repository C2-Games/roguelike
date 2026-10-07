#include "systems/movement/footprint_pathfinding.h"

#include <algorithm>
#include <queue>
#include <utility>

#include "objects/direction.h"
#include "objects/room/room.h"

namespace
{

Orientation otherOrientation(Orientation orientation)
{
  return orientation == Orientation::Horizontal ? Orientation::Vertical
                                                : Orientation::Horizontal;
}

// a footprint fits when every tile it covers is non-blocking.
bool footprintFits(const Room& room, Coordinate origin,
                   const EntitySymbol& oriented)
{
  const std::vector<Coordinate> tiles = footprintTiles(origin, oriented);
  return std::none_of(tiles.begin(), tiles.end(), [&room](Coordinate tile) {
    return isBlocking(room, tile);
  });
}

// seeds every (anchor, orientation) state that fits and is orthogonally
// adjacent to the goal -- a melee-ready posture -- rather than seeding at
// the goal cell itself, which a multi-cell footprint often can't occupy.
// the goal tile itself is never required to be non-blocking: footprints
// seek adjacency to it, not occupancy of it.
std::queue<std::pair<Coordinate, Orientation>> seedFootprintFrontier(
    const Room& room, Coordinate goal, const EntitySymbol& horizontalSymbol,
    const EntitySymbol& verticalSymbol, FootprintGoalMap& result)
{
  // computed once up front rather than re-derived per tile/direction below.
  auto orientedGrid = [&](Orientation orientation) -> const EntitySymbol& {
    return orientation == Orientation::Horizontal ? horizontalSymbol
                                                  : verticalSymbol;
  };

  // hands back a mutable reference into result for the caller to write
  // through.
  auto mapFor = [&result](Orientation orientation) -> GoalMap& {
    return orientation == Orientation::Horizontal ? result.horizontal
                                                  : result.vertical;
  };

  std::queue<std::pair<Coordinate, Orientation>> frontier;

  for (int x = 0; x < Room::WIDTH; ++x)
  {
    for (int y = 0; y < Room::HEIGHT; ++y)
    {
      Coordinate anchor(x, y);
      for (Orientation orientation :
           {Orientation::Horizontal, Orientation::Vertical})
      {
        const EntitySymbol& oriented = orientedGrid(orientation);
        Coordinate origin = originFromAnchor(anchor, oriented);
        if (!footprintFits(room, origin, oriented) ||
            !isAdjacentToFootprint(origin, oriented, goal))
        {
          continue;
        }
        mapFor(orientation)[x][y] = 0;
        frontier.emplace(anchor, orientation);
      }
    }
  }

  return frontier;
}

// expands the seeded frontier via bfs, mutating result and frontier in
// place.
void expandFootprintFrontier(
    const Room& room, const EntitySymbol& horizontalSymbol,
    const EntitySymbol& verticalSymbol, FootprintGoalMap& result,
    std::queue<std::pair<Coordinate, Orientation>>& frontier)
{
  // computed once up front rather than re-derived per tile/direction below.
  auto orientedGrid = [&](Orientation orientation) -> const EntitySymbol& {
    return orientation == Orientation::Horizontal ? horizontalSymbol
                                                  : verticalSymbol;
  };

  // hands back a mutable reference into result for the caller to write
  // through.
  auto mapFor = [&result](Orientation orientation) -> GoalMap& {
    return orientation == Orientation::Horizontal ? result.horizontal
                                                  : result.vertical;
  };

  while (!frontier.empty())
  {
    auto [anchor, orientation] = frontier.front();
    frontier.pop();
    const int nextDist = mapFor(orientation)[anchor.x][anchor.y] + 1;

    // translate: step the whole footprint one cardinal tile, orientation
    // unchanged.
    const EntitySymbol& oriented = orientedGrid(orientation);
    for (Direction direction : ALL_DIRECTIONS)
    {
      Coordinate nextAnchor = anchor + toOffset(direction);
      // a translated anchor can sit outside the room while part of its
      // footprint still reads as a valid in-bounds tile, so the anchor
      // index itself needs its own bounds check before indexing the map.
      if (!Room::inBounds(nextAnchor))
      {
        continue;
      }
      GoalMap& map = mapFor(orientation);
      if (map[nextAnchor.x][nextAnchor.y] != UNREACHABLE)
      {
        continue;
      }
      if (!footprintFits(room, originFromAnchor(nextAnchor, oriented),
                         oriented))
      {
        continue;
      }
      map[nextAnchor.x][nextAnchor.y] = nextDist;
      frontier.emplace(nextAnchor, orientation);
    }

    // rotate-in-place: toggle orientation at the same anchor.
    const Orientation rotated = otherOrientation(orientation);
    GoalMap& rotatedMap = mapFor(rotated);
    if (rotatedMap[anchor.x][anchor.y] == UNREACHABLE)
    {
      const EntitySymbol& rotatedGrid = orientedGrid(rotated);
      if (footprintFits(room, originFromAnchor(anchor, rotatedGrid),
                        rotatedGrid))
      {
        rotatedMap[anchor.x][anchor.y] = nextDist;
        frontier.emplace(anchor, rotated);
      }
    }
  }
}

}  // namespace

FootprintGoalMap computeFootprintGoalMap(const Room& room, Coordinate goal,
                                         const EntitySymbol& baseSymbol)
{
  FootprintGoalMap result{
      GoalMap(Room::WIDTH, std::vector<int>(Room::HEIGHT, UNREACHABLE)),
      GoalMap(Room::WIDTH, std::vector<int>(Room::HEIGHT, UNREACHABLE))};

  const EntitySymbol horizontalSymbol =
      orientedSymbol(baseSymbol, Orientation::Horizontal);
  const EntitySymbol verticalSymbol =
      orientedSymbol(baseSymbol, Orientation::Vertical);

  auto frontier = seedFootprintFrontier(room, goal, horizontalSymbol,
                                        verticalSymbol, result);
  expandFootprintFrontier(room, horizontalSymbol, verticalSymbol, result,
                          frontier);

  return result;
}
