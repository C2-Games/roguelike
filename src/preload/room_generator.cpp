#include "preload/room_generator.h"

#include <algorithm>
#include <memory>
#include <random>

#include "game/level_data.h"
#include "game/logger.h"
#include "game/services.h"
#include "objects/entities/enemy.h"
#include "objects/room/room.h"
#include "preload/enemy_catalog.h"

namespace
{

// rolls a fresh set of enemies for a room from its spawn table.
std::vector<std::unique_ptr<Enemy>> spawnEnemies(
    const Room& room, const std::vector<Coordinate>& enemySpawns,
    const std::vector<EnemySpawnConfig>& spawnTable,
    const EnemyCatalog& catalog, GameServices& services)
{
  std::vector<std::unique_ptr<Enemy>> enemies;

  std::vector<Coordinate> shuffledSpawns = enemySpawns;
  std::shuffle(shuffledSpawns.begin(), shuffledSpawns.end(), services.rng);

  std::size_t nextIdx = 0;
  for (const EnemySpawnConfig& entry : spawnTable)
  {
    if (nextIdx >= shuffledSpawns.size())
    {
      LOG_ERR("Room " + std::to_string(room.getRoomID()) +
              ": spawn table exceeds available spawn points; dropping "
              "remaining entries from '" +
              entry.name + "'");
      break;
    }

    const EnemyTierAttributes* attrs = catalog.find(entry.name, entry.tier);
    if (attrs == nullptr)
    {
      LOG_ERR("Room " + std::to_string(room.getRoomID()) +
              ": unknown enemy name/tier '" + entry.name + "'/" +
              std::to_string(entry.tier) + "; skipping");
      continue;
    }

    int lower = entry.range[0];
    int upper = entry.range[1];
    if (lower > upper)
    {
      LOG_ERR("Room " + std::to_string(room.getRoomID()) + ": '" + entry.name +
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

}  // namespace

namespace room_generator
{

RoomObjects generate(Room& room, const std::vector<Coordinate>& enemySpawns,
                     const std::vector<Coordinate>& /*lootSpawns*/,
                     const std::vector<Coordinate>& /*itemSpawns*/,
                     const std::vector<EnemySpawnConfig>& spawnTable,
                     const EnemyCatalog& catalog, GameServices& services)
{
  // lootSpawns/itemSpawns are unused for now -- a seam for a future
  // loot/item spawn system.
  RoomObjects objects;
  objects.enemies =
      spawnEnemies(room, enemySpawns, spawnTable, catalog, services);
  for (const auto& enemy : objects.enemies)
  {
    room.toggleOccupied(enemy->getPosition(), true);
  }
  return objects;
}

}  // namespace room_generator
