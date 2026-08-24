#include "systems/movement/move_player.h"

#include "objects/entities/player.h"
#include "objects/room/room.h"
#include "objects/tiles/tile_type.h"

namespace movement
{

PlayerStepOutcome stepPlayer(Player& player, Room& room, Coordinate direction)
{
  const Coordinate nextPos = player.getPosition() + direction;

  if (!room.isWalkable(nextPos) || room.isOccupied(nextPos))
  {
    return {PlayerStepKind::Blocked, Coordinate()};
  }

  if (room.getTileType(nextPos) == TileType::Door)
  {
    return {PlayerStepKind::AtDoor, nextPos};
  }

  const Coordinate oldPos = player.getPosition();
  room.toggleOccupied(oldPos, false);
  player.moveTo(nextPos);
  room.toggleOccupied(nextPos, true);
  return {PlayerStepKind::Moved, Coordinate()};
}

}  // namespace movement
