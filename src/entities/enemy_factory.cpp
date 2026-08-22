#include "entities/enemy_factory.h"

#include <algorithm>
#include <random>

#include "core/logger.h"
#include "core/services.h"
#include "entities/enemy.h"
#include "preload/enemy_catalog.h"
#include "preload/level_loader.h"
#include "world/map/room.h"

namespace enemy_factory
{

std::vector<std::unique_ptr<Enemy>> rollForRoom(
    const Room& room, const std::vector<EnemySpawnConfig>& spawnTable,
    const EnemyCatalog& catalog, GameServices& services)
{
  std::vector<std::unique_ptr<Enemy>> enemies;

  std::vector<Coordinate> shuffledSpawns = room.enemySpawns;
  std::shuffle(shuffledSpawns.begin(), shuffledSpawns.end(), services.rng);

  std::size_t nextIdx = 0;
  for (const EnemySpawnConfig& entry : spawnTable)
  {
    if (nextIdx >= shuffledSpawns.size())
    {
      LOG_ERR("Room " + std::to_string(room.roomID) +
              ": spawn table exceeds available spawn points; dropping "
              "remaining entries from '" +
              entry.name + "'");
      break;
    }

    const EnemyTierAttributes* attrs = catalog.find(entry.name, entry.tier);
    if (attrs == nullptr)
    {
      LOG_ERR("Room " + std::to_string(room.roomID) +
              ": unknown enemy name/tier '" + entry.name + "'/" +
              std::to_string(entry.tier) + "; skipping");
      continue;
    }

    int lower = entry.range[0];
    int upper = entry.range[1];
    if (lower > upper)
    {
      LOG_ERR("Room " + std::to_string(room.roomID) + ": '" + entry.name +
              "' has an inverted range; swapping");
      std::swap(lower, upper);
    }
    int count = std::uniform_int_distribution<int>(lower, upper)(services.rng);
    for (int i = 0; i < count && nextIdx < shuffledSpawns.size();
         ++i, ++nextIdx)
    {
      const Coordinate& pos = shuffledSpawns[nextIdx];
      enemies.push_back(std::make_unique<Enemy>(
          pos, attrs->fov->clone(), attrs->symbol, attrs->health, attrs->speed,
          attrs->attackDamage, attrs->chaseMemoryDuration));
    }
  }

  return enemies;
}

}  // namespace enemy_factory
