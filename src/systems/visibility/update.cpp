#include "systems/visibility/update.h"

#include "systems/visibility/delta.h"
#include "systems/visibility/recompute.h"

namespace visibility
{

void update(Room& room, Coordinate origin, const FOV& fov)
{
  recompute(room, origin, fov);
}

void update(Room& room, Coordinate previousOrigin, Coordinate origin,
            const FOV& fov)
{
  if (!applyDelta(room, previousOrigin, origin, fov))
  {
    recompute(room, origin, fov);
  }
}

}  // namespace visibility
