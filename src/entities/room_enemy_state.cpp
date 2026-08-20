#include "entities/room_enemy_state.h"

#include <algorithm>

void RoomEnemyState::ensureSpawned(
    const Room& room, const std::vector<EnemySpawnConfig>& spawnTable,
    const EnemyCatalog& catalog, GameServices& services)
{
  if (spawned_)
  {
    return;
  }
  spawned_ = true;
  enemies_ = rollEnemiesForRoom(room, spawnTable, catalog, services);
}

Enemy* RoomEnemyState::enemyAt(Coordinate pos, const Enemy* exclude) const
{
  auto hit = std::find_if(enemies_.begin(), enemies_.end(),
                          [&](const std::unique_ptr<Enemy>& enemy) {
                            return enemy.get() != exclude && enemy->isAlive() &&
                                   enemy->getPosition() == pos;
                          });
  return hit != enemies_.end() ? hit->get() : nullptr;
}

void RoomEnemyState::reap(std::vector<std::unique_ptr<Enemy>>& active)
{
  active.erase(std::remove_if(active.begin(), active.end(),
                              [](const std::unique_ptr<Enemy>& e) {
                                return !e->isAlive();
                              }),
               active.end());
}
