#ifndef RECOMPUTE_H
#define RECOMPUTE_H

#include "objects/coordinate.h"

struct FOV;
struct Room;

namespace visibility
{

/**
 * @brief Recompute a room's per-tile visibility from a viewer position, as
 * a full sweep.
 *
 * @param room Room whose tile grid is updated.
 * @param origin World position of the viewer (typically the player).
 * @param fov Precomputed FoV mask defining which offsets are lit.
 */
void recompute(Room& room, Coordinate origin, const FOV& fov);

}  // namespace visibility

#endif
