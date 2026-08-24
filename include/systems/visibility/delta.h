#ifndef DELTA_H
#define DELTA_H

#include "objects/coordinate.h"

struct FOV;
struct Room;

namespace visibility
{

/**
 * @brief Incrementally update visibility for a single unit-cardinal step.
 *
 * @param room Room whose tile grid is updated.
 * @param previousOrigin Viewer position before the step.
 * @param origin Viewer position after the step.
 * @param fov FoV mask, unchanged since the previous frame.
 * @return bool True if applied; false if the step wasn't a precomputed unit
 * cardinal move, meaning the caller must fall back to a full recompute.
 */
bool applyDelta(Room& room, Coordinate previousOrigin, Coordinate origin,
                const FOV& fov);

}  // namespace visibility

#endif
