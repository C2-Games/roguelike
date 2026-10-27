#include "preload/room_generator.h"

#include <algorithm>
#include <cstdlib>
#include <memory>
#include <optional>
#include <random>
#include <set>

#include "game/level_data.h"
#include "game/logger.h"
#include "game/services.h"
#include "objects/entities/enemy.h"
#include "objects/entities/entity_symbol.h"
#include "objects/room/room.h"
#include "preload/enemy_catalog.h"

namespace
{

// a footprint origin paired with the full footprint found there, so callers
// don't need to recompute footprintTiles for the winning candidate.
struct SpawnOrigin
{
  Coordinate origin;
  std::vector<Coordinate> footprint;
};

/**
 * @brief Search a small radius around a desired spawn point for a fully
 * clear, unclaimed footprint.
 *
 * @param room Room whose tile grid/occupancy is checked.
 * @param desired The spawn table's original spawn point.
 * @param symbol The spawning enemy's (always-Horizontal-authored, already
 * rectangular) symbol grid.
 * @param claimed Tiles already claimed earlier in this spawn batch (not yet
 * reflected in `room`'s own occupancy, which is only toggled after the
 * whole batch is placed).
 * @return The first footprint origin found whose full footprint is Floor,
 * unoccupied, and unclaimed, searching outward ring-by-ring from `desired`,
 * paired with that footprint; `std::nullopt` if none is found within the
 * search radius.
 */
std::optional<SpawnOrigin> findSpawnOrigin(const Room& room, Coordinate desired,
                                           const EntitySymbol& symbol,
                                           const std::set<Coordinate>& claimed)
{
  constexpr int SEARCH_RADIUS = 2;
  for (int radius = 0; radius <= SEARCH_RADIUS; ++radius)
  {
    for (int dx = -radius; dx <= radius; ++dx)
    {
      for (int dy = -radius; dy <= radius; ++dy)
      {
        if (std::max(std::abs(dx), std::abs(dy)) != radius)
        {
          continue;  // only this ring -- closer candidates already tried.
        }
        const Coordinate candidate = desired + Coordinate(dx, dy);
        const std::vector<Coordinate> footprint =
            footprintTiles(candidate, symbol);
        const bool clear = std::all_of(
            footprint.begin(), footprint.end(), [&](Coordinate tile) {
              return room.getTileType(tile) == TileType::Floor &&
                     !room.isOccupied(tile) && !claimed.contains(tile);
            });
        if (clear)
        {
          return SpawnOrigin{candidate, footprint};
        }
      }
    }
  }
  return std::nullopt;
}

// rolls a fresh set of enemies for a room from its spawn table.
std::vector<std::unique_ptr<Enemy>> spawnEnemies(
    const Room& room, const std::vector<Coordinate>& enemySpawns,
    const std::vector<EnemySpawnConfig>& spawnTable,
    const EnemyCatalog& catalog, GameServices& services)
{
  std::vector<std::unique_ptr<Enemy>> enemies;

  std::vector<Coordinate> shuffledSpawns = enemySpawns;
  std::shuffle(shuffledSpawns.begin(), shuffledSpawns.end(), services.rng);

  // room.isOccupied isn't populated until generate() finishes placing the
  // whole batch, so intra-batch footprint overlap needs its own tracking.
  std::set<Coordinate> claimed;

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
      const Coordinate& desired = shuffledSpawns[nextIdx];
      std::optional<SpawnOrigin> spawn =
          findSpawnOrigin(room, desired, attrs->symbol, claimed);
      if (!spawn.has_value())
      {
        LOG_ERR("Room " + std::to_string(room.getRoomID()) + ": '" +
                entry.name +
                "' found no clear footprint near spawn point; "
                "skipping");
        continue;
      }
      for (Coordinate tile : spawn->footprint)
      {
        claimed.insert(tile);
      }
      enemies.push_back(std::make_unique<Enemy>(
          spawn->origin, attrs->fov->clone(), attrs->symbol, attrs->health,
          attrs->speed, attrs->attackDamage, attrs->chaseMemoryDuration));
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
    for (Coordinate tile :
         footprintTiles(enemy->getPosition(), enemy->getSymbol()))
    {
      room.toggleOccupied(tile, true);
    }
  }
  return objects;
}

}  // namespace room_generator
