#include "core/room_enemy_logic.h"

#include <algorithm>

#include "core/enemy_factory.h"
#include "objects/entities/enemy.h"
#include "objects/room/room.h"

namespace room_enemy_logic
{

void ensureSpawned(Room& room, const std::vector<EnemySpawnConfig>& spawnTable,
                   const EnemyCatalog& catalog, GameServices& services)
{
  if (room.enemyState.spawned)
  {
    return;
  }
  room.enemyState.spawned = true;
  room.enemyState.enemies =
      enemy_factory::rollForRoom(room, spawnTable, catalog, services);
}

void reap(std::vector<std::unique_ptr<Enemy>>& active)
{
  active.erase(std::remove_if(active.begin(), active.end(),
                              [](const std::unique_ptr<Enemy>& e) {
                                return !e->isAlive();
                              }),
               active.end());
}

}  // namespace room_enemy_logic
