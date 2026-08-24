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
    room.toggleReveal(pos, false);
  }

  for (const Coordinate& offset : *entering)
  {
    Coordinate pos = origin + offset;
    room.toggleReveal(pos, true);
  }

  return true;
}

}  // namespace visibility
