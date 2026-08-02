#ifndef VISIBILITY_H
#define VISIBILITY_H

#include "core/coordinate.h"

class FOV;
struct Room;

namespace visibility {

/**
 * @brief Recompute per-tile visibility for `room` from `origin`.
 *
 * @param room   Room whose visibility state gets refreshed in place.
 * @param origin World position of the viewer (typically player).
 * @param fov    Precomputed FoV mask defining which offsets are lit.
 */
void update(Room& room, Coordinate origin, const FOV& fov);

}  // namespace visibility

#endif
