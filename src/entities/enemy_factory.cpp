#include "entities/enemy_factory.h"

#include <algorithm>
#include <random>

#include "core/logger.h"
#include "core/services.h"
#include "entities/enemy.h"
#include "entities/enemy_catalog.h"
#include "world/map/level_config.h"
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

    if (!std::bernoulli_distribution(entry.probDist)(services.rng))
    {
      continue;
    }

    const EnemyTierAttributes* attrs = catalog.find(entry.name, entry.tier);
    if (attrs == nullptr)
    {
      LOG_ERR("Room " + std::to_string(room.roomID) +
              ": unknown enemy name/tier '" + entry.name + "'/" +
              std::to_string(entry.tier) + "; skipping");
      continue;
    }

    int lo = entry.min;
    int hi = entry.max;
    if (lo > hi)
    {
      LOG_ERR("Room " + std::to_string(room.roomID) + ": '" + entry.name +
              "' has min > max; swapping");
      std::swap(lo, hi);
    }

    int count = std::uniform_int_distribution<int>(lo, hi)(services.rng);
    for (int i = 0; i < count && nextIdx < shuffledSpawns.size();
         ++i, ++nextIdx)
    {
      const Coordinate& pos = shuffledSpawns[nextIdx];
      enemies.push_back(std::make_unique<Enemy>(
          pos.x, pos.y, attrs->symbol, attrs->health, attrs->speed,
          attrs->attackDamage, attrs->fov, attrs->chaseMemoryDuration));
    }
  }

  return enemies;
}

}  // namespace enemy_factory
