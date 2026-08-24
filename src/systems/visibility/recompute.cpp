#include "systems/visibility/recompute.h"

#include "objects/fovs/fov.h"
#include "objects/room/room.h"

namespace visibility
{

void recompute(Room& room, Coordinate origin, const FOV& fov)
{
  room.clearVisible();
  for (const Coordinate& pos : fov.absoluteFOV(origin))
  {
    room.reveal(pos.x, pos.y);
  }
}

}  // namespace visibility
