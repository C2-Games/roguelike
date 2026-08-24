#ifndef UPDATE_H
#define UPDATE_H

#include "objects/coordinate.h"

struct FOV;
struct Room;

namespace visibility
{

// recompute a room's per-tile visibility from a viewer position.
void update(Room& room, Coordinate origin, const FOV& fov);

// update visibility for a step, falling back to a full recompute.
void update(Room& room, Coordinate previousOrigin, Coordinate origin,
            const FOV& fov);

}  // namespace visibility

#endif
