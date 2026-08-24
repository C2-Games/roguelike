#include "systems/visibility/delta.h"

#include <vector>

#include "objects/fovs/fov.h"
#include "objects/room/room.h"

namespace visibility
{

bool applyDelta(Room& room, Coordinate previousOrigin, Coordinate origin,
                const FOV& fov)
{
  Coordinate dir = origin - previousOrigin;
  const std::vector<Coordinate>* leaving = fov.leavingOffsets(dir);
  const std::vector<Coordinate>* entering =
      fov.leavingOffsets(Coordinate(-dir.x, -dir.y));
  if (leaving == nullptr || entering == nullptr)
  {
    return false;
  }

  for (const Coordinate& offset : *leaving)
  {
    Coordinate pos = previousOrigin + offset;
    if (pos.x < 0 || pos.x >= Room::WIDTH || pos.y < 0 || pos.y >= Room::HEIGHT)
    {
      continue;
    }
    // No bounds-checked equivalent of reveal() exists for clearing a single
    // tile, so this open-codes the same bounds check reveal() does.
    room.tiles[pos.x][pos.y].clearVisible();
  }

  for (const Coordinate& offset : *entering)
  {
    Coordinate pos = origin + offset;
    room.reveal(pos.x, pos.y);
  }

  return true;
}

}  // namespace visibility
