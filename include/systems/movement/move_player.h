#ifndef MOVE_PLAYER_H
#define MOVE_PLAYER_H

#include <cstdint>

#include "objects/coordinate.h"

class Player;
struct Room;

namespace movement
{

// outcome of a single attempted player step.
enum class PlayerStepKind : std::uint8_t
{
  Blocked,  // no-op: out of bounds, a wall, or another enemy.
  Moved,    // player already relocated to the new tile.
  AtDoor,   // player left in place; caller resolves the room transition.
};

struct PlayerStepOutcome
{
  PlayerStepKind kind;
  Coordinate doorPos;  // valid only when kind == AtDoor.
};

/**
 * @brief Attempt to step the player one tile in `direction`.
 *
 * @param player Player to move.
 * @param room Room the player occupies, for wall/occupancy queries.
 * @param direction Unit offset to attempt to move by.
 * @return Blocked (no-op), Moved (player already relocated), or AtDoor
 * (player left on their current tile; the caller resolves the room
 * transition, or calls player.moveTo() itself for an unlinked door).
 */
PlayerStepOutcome stepPlayer(Player& player, Room& room, Coordinate direction);

}  // namespace movement

#endif
