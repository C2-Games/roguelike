#include "core/room_enemy_logic.h"

#include <algorithm>

#include "objects/entities/enemy.h"

namespace room_enemy_logic
{

void reap(std::vector<std::unique_ptr<Enemy>>& active)
{
  active.erase(std::remove_if(active.begin(), active.end(),
                              [](const std::unique_ptr<Enemy>& e) {
                                return !e->isAlive();
                              }),
               active.end());
}

}  // namespace room_enemy_logic
