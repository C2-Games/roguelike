#ifndef ROOM_GENERATOR_H
#define ROOM_GENERATOR_H

#include <array>
#include <string>
#include <vector>

#include "objects/coordinate.h"

struct Room;
struct RoomObjects;
struct GameServices;
class EnemyCatalog;

// one enemy spawn-table entry from a room's JSON metadata.
struct EnemySpawnConfig
{
  std::string name;
  int tier = 0;
  std::array<int, 2> range{};
};

namespace room_generator
{

/**
 * @brief Roll a room's live entities from its authored spawn points.
 *
 * @param room Room whose spawn points are being populated; occupancy is
 *             toggled for each spawned enemy.
 * @param enemySpawns Authored enemy spawn point positions.
 * @param lootSpawns Authored loot spawn point positions.
 * @param itemSpawns Authored item spawn point positions.
 * @param spawnTable Enemy counts/ranges to roll against, from room config.
 * @param catalog Resolved enemy name/tier attributes.
 * @param services RNG source for the roll.
 * @return The room's freshly rolled live entities.
 */
RoomObjects generate(Room& room, const std::vector<Coordinate>& enemySpawns,
                     const std::vector<Coordinate>& lootSpawns,
                     const std::vector<Coordinate>& itemSpawns,
                     const std::vector<EnemySpawnConfig>& spawnTable,
                     const EnemyCatalog& catalog, GameServices& services);

}  // namespace room_generator

#endif
