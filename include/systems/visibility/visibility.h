#ifndef VISIBILITY_H
#define VISIBILITY_H

#include "objects/coordinate.h"

struct FOV;
struct Room;

namespace visibility
{

/**
 * @brief Recompute a room's per-tile visibility from a viewer position.
 *
 * @param room Room whose tile grid is updated.
 * @param origin World position of the viewer (typically the player).
 * @param fov Precomputed FoV mask defining which offsets are lit.
 */
void update(Room& room, Coordinate origin, const FOV& fov);

/**
 * @brief Update visibility for a step, falling back to a full recompute.
 *
 * @param room Room whose tile grid is updated.
 * @param previousOrigin Viewer position before the step.
 * @param origin Viewer position after the step.
 * @param fov FoV mask, unchanged since the previous frame.
 */
void update(Room& room, Coordinate previousOrigin, Coordinate origin,
            const FOV& fov);

}  // namespace visibility

#endif
