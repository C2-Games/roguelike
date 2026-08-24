#ifndef LEVEL_LOADER_H
#define LEVEL_LOADER_H

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "objects/room/room_types.h"

// one door-to-door link between two rooms, currently bidirectional
// (fromRoom->toRoom and toRoom->fromRoom)
struct RoomAdjacency
{
  int fromRoom;
  DoorNumber fromDoor;
  int toRoom;
  DoorNumber toDoor;
};

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

class Level;
struct GameServices;
class EnemyCatalog;

namespace level_loader
{

/**
 * @brief Load a level directory (level.json, map.json, and every room's
 * JSON metadata + referenced .txt template), wire the room graph, spawn
 * every room's enemies, and seal any doors left unlinked by this level's
 * adjacency back to Wall tiles.
 *
 * @param levelDir Directory containing the level's config files.
 * @param services Shared services; used transiently to roll each room's
 *                 enemy spawns.
 * @param catalog  Resolves each room's spawn-table entries to stats.
 * @return A fully-built Level.
 */
Level loadLevel(const std::filesystem::path& levelDir, GameServices& services,
                const EnemyCatalog& catalog);

}  // namespace level_loader

#endif
