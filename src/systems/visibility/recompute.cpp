#include "systems/visibility/recompute.h"

#include <cmath>

#include "objects/direction.h"
#include "objects/fovs/fov.h"
#include "objects/room/room.h"

namespace
{

Coordinate toWorldPosition(Direction direction, Coordinate origin, int depth,
                           int col)
{
  switch (direction)
  {
    case Direction::North:
      return origin + Coordinate(col, -depth);
    case Direction::South:
      return origin + Coordinate(col, depth);
    case Direction::East:
      return origin + Coordinate(depth, col);
    case Direction::West:
      return origin + Coordinate(-depth, col);
  }
  return origin;
}

struct Row
{
  int depth;
  float startSlope;
  float endSlope;
};

// slope from the origin through a tile's leading edge; used both to open a
// newly-visible row and to close one where a wall was just hit.
float edgeSlope(int depth, int col)
{
  return (static_cast<float>(2 * col) - 1.0F) /
         (2.0F * static_cast<float>(depth));
}

int minCol(const Row& row)
{
  return static_cast<int>(
      std::floor((static_cast<float>(row.depth) * row.startSlope) + 0.5F));
}

int maxCol(const Row& row)
{
  return static_cast<int>(
      std::ceil((static_cast<float>(row.depth) * row.endSlope) - 0.5F));
}

// a tile is visible only if its own center lies within this row's slope
// window, rather than inferring visibility from a neighboring cell's
// pass/fail history the way naive octant shadowcasting does; that's what
// keeps shadows behind off-axis single-tile obstacles from narrowing and
// vanishing a few tiles out.
bool isSymmetric(const Row& row, int col)
{
  float depth = static_cast<float>(row.depth);
  return static_cast<float>(col) >= depth * row.startSlope &&
         static_cast<float>(col) <= depth * row.endSlope;
}

void scanRow(Room& room, Coordinate origin, const FOV& fov, Direction direction,
             Row row)
{
  int radius = fov.maxRadius();
  bool hasPrev = false;
  bool prevWasWall = false;

  for (int col = minCol(row); col <= maxCol(row); ++col)
  {
    Coordinate pos = toWorldPosition(direction, origin, row.depth, col);
    bool isWall = !room.isWalkable(pos);

    if ((isWall || isSymmetric(row, col)) && fov.in(origin, pos))
    {
      room.toggleReveal(pos, true);
    }

    if (hasPrev)
    {
      if (prevWasWall && !isWall)
      {
        row.startSlope = edgeSlope(row.depth, col);
      }
      else if (!prevWasWall && isWall && row.depth < radius)
      {
        scanRow(room, origin, fov, direction,
                Row{row.depth + 1, row.startSlope, edgeSlope(row.depth, col)});
      }
    }

    prevWasWall = isWall;
    hasPrev = true;
  }

  if (hasPrev && !prevWasWall && row.depth < radius)
  {
    scanRow(room, origin, fov, direction,
            Row{row.depth + 1, row.startSlope, row.endSlope});
  }
}

}  // namespace

namespace visibility
{

void recompute(Room& room, Coordinate origin, const FOV& fov)
{
  room.clearVisible();
  room.toggleReveal(origin, true);

  for (Direction direction : ALL_DIRECTIONS)
  {
    scanRow(room, origin, fov, direction, Row{1, -1.0F, 1.0F});
  }
}

}  // namespace visibility
