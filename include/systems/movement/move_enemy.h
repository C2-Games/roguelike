#ifndef MOVE_ENEMY_H
#define MOVE_ENEMY_H

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
bool advanceEnemy(Enemy& enemy, const Player& player, Room& room,
                  const GoalMapCache& cache, GameServices& services);

}  // namespace movement

#endif
