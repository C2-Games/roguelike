#ifndef LEVEL_CONFIG_H
#define LEVEL_CONFIG_H

#include <array>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

class Enemy;
class EnemyCatalog;
struct GameServices;
struct Room;

// Door number based on the order of doors in the room's template file
using DoorNumber = int;

// one door-to-door link between two rooms, currently bidirectional
// (fromRoom->toRoom and toRoom->fromRoom)
struct RoomAdjacency
{
  int fromRoom;
  DoorNumber fromDoor;
  int toRoom;
  DoorNumber toDoor;
};

/** @brief One enemy spawn-table entry from a room's JSON metadata. */
struct EnemySpawnConfig
{
  std::string name;
  int tier;
  std::array<int, 2> range;
};

/**
 * @brief Roll a fresh set of enemies for a room from its spawn table.
 *
 * @param room       The room to populate; provides enemy spawn points.
 * @param spawnTable The room's authored enemy entries (name/tier/range).
 * @param catalog    Resolves each entry's name/tier to stats.
 * @param services   RNG source.
 * @return Owning vector of newly-created enemies.
 */
std::vector<std::unique_ptr<Enemy>> rollEnemiesForRoom(
    const Room& room, const std::vector<EnemySpawnConfig>& spawnTable,
    const EnemyCatalog& catalog, GameServices& services);

/** @brief One room's metadata, parsed from room_<id>.json. */
struct RoomConfig
{
  int id;
  std::string name;
  std::string ref;  // filename under assets/rooms/
  std::vector<EnemySpawnConfig> enemies;
};

/** @brief Level-wide metadata, parsed from level.json. */
struct LevelMeta
{
  int id;
  std::string name;
  std::string description;
  int roomCount;
  int startRoomID;
  int bossRoomID;
};

/** @brief Fully parsed contents of a level directory. */
struct LevelConfig
{
  LevelMeta meta;
  std::vector<RoomAdjacency> adjacency;  // from map.json
  std::map<int, RoomConfig> rooms;       // id -> per-room config
};

/**
 * @brief Load and cross-validate an entire level directory (level.json,
 * map.json, and every room_<id>.json map.json references).
 *
 * @param levelDir Directory containing the level's config files.
 * @throws std::runtime_error on any missing file, malformed JSON, or
 *         cross-reference mismatch (id mismatches, room-count mismatches).
 */
LevelConfig loadLevelConfig(const std::filesystem::path& levelDir);

#endif
