#include "entities/enemy_registry.h"

#include <algorithm>
#include <utility>

#include "entities/enemy.h"
#include "entities/enemy_factory.h"

EnemyRegistry::EnemyRegistry(GameServices& services) : services_(services) {}

void EnemyRegistry::ensureSpawned(int roomID, const Room& room) {
  if (visited_[roomID]) return;
  visited_[roomID] = true;
  perRoom_[roomID] = enemy_factory::rollForRoom(room, services_);
}

void EnemyRegistry::loadForRoom(int roomID, const Room& room,
                                std::vector<std::unique_ptr<Enemy>>& active) {
  ensureSpawned(roomID, room);
  active = std::move(perRoom_[roomID]);
}

void EnemyRegistry::transitionActive(
    int fromRoomID, int toRoomID, const Room& toRoom,
    std::vector<std::unique_ptr<Enemy>>& active) {
  // Park the current room's enemies back into storage.
  perRoom_[fromRoomID] = std::move(active);
  // Load the destination room, spawning fresh enemies on first visit.
  ensureSpawned(toRoomID, toRoom);
  active = std::move(perRoom_[toRoomID]);
}

void EnemyRegistry::reap(std::vector<std::unique_ptr<Enemy>>& active) {
  // Remove dead enemies from the active list.
  active.erase(std::remove_if(active.begin(), active.end(),
                              [](const std::unique_ptr<Enemy>& e) {
                                return !e->isAlive();
                              }),
               active.end());
}
