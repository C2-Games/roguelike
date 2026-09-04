#include "systems/visibility/delta.h"

#include <algorithm>
#include <vector>

#include "objects/fovs/fov.h"
#include "objects/room/room.h"

namespace
{

// walls within FOV range invalidate the offset-based leaving/entering diff,
// since a step near a wall/corner can change visibility for tiles well
// outside those rings; the caller must fall back to a full recompute.
bool anyBlocked(const Room& room, const FOV& fov, Coordinate origin)
{
  std::vector<Coordinate> positions = fov.absoluteFOV(origin);
  return std::any_of(
      positions.begin(), positions.end(),
      [&room](const Coordinate& pos) { return !room.isWalkable(pos); });
}

}  // namespace

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

  if (anyBlocked(room, fov, previousOrigin) || anyBlocked(room, fov, origin))
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
