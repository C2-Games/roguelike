#include "entities/room_enemy_state.h"

#include <algorithm>

#include "entities/enemy_factory.h"

void RoomEnemyState::ensureSpawned(
    const Room& room, const std::vector<EnemySpawnConfig>& spawnTable,
    const EnemyCatalog& catalog, GameServices& services) {
  if (spawned_) {
    return;
  }
  spawned_ = true;
  enemies_ = enemy_factory::rollForRoom(room, spawnTable, catalog, services);
}

void RoomEnemyState::reap(std::vector<std::unique_ptr<Enemy>>& active) {
  active.erase(std::remove_if(active.begin(), active.end(),
                              [](const std::unique_ptr<Enemy>& e) {
                                return !e->isAlive();
                              }),
               active.end());
}
