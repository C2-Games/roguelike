#ifndef MOVEMENT_H
#define MOVEMENT_H

#include <cstdint>

#include "objects/coordinate.h"
#include "systems/movement/goal_map_cache.h"

class Enemy;
class Player;
struct Room;
struct GameServices;

namespace movement
{

/**
 * @brief Advance one enemy's AI/movement for this frame: refreshes its
 * chase state, paths toward its current target (or wanders), and resolves
 * any resulting melee attack.
 *
 * @param enemy Enemy to advance.
 * @param player Player the enemy may chase/attack.
 * @param room Room the enemy occupies, for wall/occupancy queries.
 * @param cache Goal-map cache used to path toward the chase target.
 * @param services RNG source for movement tiebreaks and wandering.
 * @return True when the enemy attacks the player this frame.
 */
bool advanceEnemy(Enemy& enemy, const Player& player, const Room& room,
                  const GoalMapCache& cache, GameServices& services);

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
PlayerStepOutcome stepPlayer(Player& player, const Room& room,
                             Coordinate direction);

}  // namespace movement

#endif
