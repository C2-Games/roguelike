#include "entities/enemy_factory.h"

#include <random>

#include "core/coordinate.h"
#include "core/services.h"
#include "entities/enemy.h"
#include "world/map/room.h"

namespace enemy_factory {

std::vector<std::unique_ptr<Enemy>> rollForRoom(const Room& room,
                                                GameServices& services) {
  std::vector<std::unique_ptr<Enemy>> enemies;

  // Each authored spawn point independently has a 50% chance of producing an
  // enemy. Placeholder odds until per-room spawn tables exist.
  std::bernoulli_distribution shouldSpawn(0.5);

  // Alternate placeholder types by spawn-list order: goblin, ogre, goblin,
  // ogre, ... Real per-spawn enemy tables will replace this once the room
  // config format lands.
  for (std::size_t i = 0; i < room.enemySpawns.size(); ++i) {
    if (!shouldSpawn(services.rng)) continue;
    const Coordinate& pos = room.enemySpawns[i];
    char symbol = (i % 2 == 0) ? 'G' : 'O';
    enemies.push_back(std::make_unique<Enemy>(pos.x, pos.y, symbol));
  }

  return enemies;
}

}  // namespace enemy_factory
