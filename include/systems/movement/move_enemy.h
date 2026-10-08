#ifndef MOVE_ENEMY_H
#define MOVE_ENEMY_H

#include "systems/movement/footprint_goal_map_cache.h"
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
 * @param footprintCache Footprint-aware goal-map cache used to path
 * multi-tile enemies toward the chase target.
 * @param services RNG source for movement tiebreaks and wandering.
 * @return True when the enemy is in AIState::Attack range this frame and
 * wants to fire (the caller should ask systems/combat to spawn a
 * projectile); false when the enemy moved or held per its non-Attack state.
 */
bool advanceEnemy(Enemy& enemy, const Player& player, Room& room,
                  const GoalMapCache& cache,
                  const FootprintGoalMapCache& footprintCache,
                  GameServices& services);

}  // namespace movement

#endif
