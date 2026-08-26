#include "systems/visibility/recompute.h"

#include "objects/fovs/fov.h"
#include "objects/room/room.h"

namespace
{

// standard octant transform table for recursive shadowcasting: each column
// gives (xx, xy, yx, yy) mapping a (row, col) scan into a real grid offset
// for one of the 8 octants.
constexpr int MULT[4][8] = {
    {1, 0, 0, -1, -1, 0, 0, 1},
    {0, 1, -1, 0, 0, -1, 1, 0},
    {0, 1, 1, 0, 0, -1, -1, 0},
    {1, 0, 0, 1, -1, 0, 0, -1},
};

void castLight(Room& room, Coordinate origin, const FOV& fov, int row,
               float startSlope, float endSlope, int radius, int xx, int xy,
               int yx, int yy)
{
  if (startSlope < endSlope)
  {
    return;
  }

  float nextStartSlope = startSlope;
  for (int currentRow = row; currentRow <= radius; ++currentRow)
  {
    int deltaY = -currentRow;
    bool blocked = false;
    for (int deltaX = -currentRow; deltaX <= 0; ++deltaX)
    {
      float leftSlope = (static_cast<float>(deltaX) - 0.5F) /
                        (static_cast<float>(deltaY) + 0.5F);
      float rightSlope = (static_cast<float>(deltaX) + 0.5F) /
                         (static_cast<float>(deltaY) - 0.5F);
      if (rightSlope > startSlope)
      {
        continue;
      }
      if (leftSlope < endSlope)
      {
        break;
      }

      Coordinate pos = origin + Coordinate((deltaX * xx) + (deltaY * xy),
                                           (deltaX * yx) + (deltaY * yy));

      if (fov.in(origin, pos))
      {
        room.toggleReveal(pos, true);
      }

      if (blocked)
      {
        if (!room.isWalkable(pos))
        {
          nextStartSlope = rightSlope;
          continue;
        }
        blocked = false;
        startSlope = nextStartSlope;
      }
      else if (!room.isWalkable(pos) && currentRow < radius)
      {
        blocked = true;
        castLight(room, origin, fov, currentRow + 1, startSlope, leftSlope,
                  radius, xx, xy, yx, yy);
        nextStartSlope = rightSlope;
      }
    }

    if (blocked)
    {
      break;
    }
  }
}

}  // namespace

namespace visibility
{

void recompute(Room& room, Coordinate origin, const FOV& fov)
{
  room.clearVisible();
  room.toggleReveal(origin, true);

  int radius = fov.maxRadius();
  for (int octant = 0; octant < 8; ++octant)
  {
    castLight(room, origin, fov, 1, 1.0F, 0.0F, radius, MULT[0][octant],
              MULT[1][octant], MULT[2][octant], MULT[3][octant]);
  }
}

}  // namespace visibility
