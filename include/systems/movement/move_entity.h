#ifndef MOVE_ENTITY_H
#define MOVE_ENTITY_H

#include "objects/coordinate.h"
#include "objects/entities/entity_symbol.h"

class Entity;

namespace movement
{

/**
 * @brief Commit a throttled position and orientation change on an entity.
 *
 * @param entity Entity to move; its own speed gates whether this frame
 * actually commits.
 * @param newPos Position to move to once the throttle admits it.
 * @param orientation Orientation to move to once the throttle admits it.
 */
void moveEntity(Entity& entity, Coordinate newPos, Orientation orientation);

/**
 * @brief Commit a throttled position change on an entity, preserving its
 * current orientation.
 *
 * @param entity Entity to move; its own speed gates whether this frame
 * actually commits.
 * @param newPos Position to move to once the throttle admits it.
 */
void moveEntity(Entity& entity, Coordinate newPos);

}  // namespace movement

#endif
