#ifndef VISIBILITY_H
#define VISIBILITY_H

#include "core/coordinate.h"

class FOV;
struct Room;

/**
 * @brief Free functions that update per-tile visibility on a Room.
 *
 * This is all visibility logic for Rooms, separated from the Room data
 * structure itself. Provides functions to update which tiles are visible and
 * explored based on a viewer's field of view.
 */
namespace visibility {

/**
 * @brief Recompute per-tile visibility for `room` from `origin`.
 *
 * Clears the room's `visible` bitmap, then walks the FoV's absolute offsets
 * from `origin` and marks each covered tile as visible (and permanently
 * explored if the tile is not Void) via Room::reveal. reveal() is
 * bounds-checked, so out-of-room FoV offsets are safely ignored.
 *
 * Typical caller cadence: once per frame in the update phase, before render.
 *
 * @param room   Room whose visibility state gets refreshed in place.
 * @param origin World position of the viewer (typically player).
 * @param fov    Precomputed FoV mask defining which offsets are lit.
 */
void update(Room& room, Coordinate origin, const FOV& fov);

}  // namespace visibility

#endif
