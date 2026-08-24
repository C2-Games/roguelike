#ifndef ROOM_ENEMY_LOGIC_H
#define ROOM_ENEMY_LOGIC_H

#include <memory>
#include <vector>

class Enemy;

// stopgap home for reap logic that reaches into Enemy -- belongs in
// systems/combat.h or systems/movement.h once the rest of the systems/
// batch exists.
namespace room_enemy_logic
{

/** @brief Drop dead enemies from `active`. */
void reap(std::vector<std::unique_ptr<Enemy>>& active);

}  // namespace room_enemy_logic

#endif
