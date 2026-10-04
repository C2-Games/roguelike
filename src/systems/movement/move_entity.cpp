#include "systems/movement/move_entity.h"

#include "objects/entities/entity.h"

namespace movement
{

void moveEntity(Entity& entity, Coordinate newPos, Orientation orientation)
{
  if (!entity.tickMovementFrame())
  {
    return;
  }

  if (newPos != entity.getPosition() || orientation != entity.getOrientation())
  {
    entity.setActionState(EntityActionState::Move);
  }
  entity.setPosition(newPos);
  entity.setOrientation(orientation);
}

void moveEntity(Entity& entity, Coordinate newPos)
{
  moveEntity(entity, newPos, entity.getOrientation());
}

}  // namespace movement
