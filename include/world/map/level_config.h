#ifndef LEVEL_CONFIG_H
#define LEVEL_CONFIG_H

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "core/coordinate.h"

struct Room;

/** @brief A compass direction a room's door faces. */
enum class Direction
{
  North,
  East,
  South,
  West
};

/** @brief One outgoing connection from a room, parsed from map.json. */
struct RoomEdge
{
  Direction direction;
  int to;
};

/** @brief One enemy spawn-table entry from a room's JSON metadata. */
struct EnemySpawnConfig
{
  std::string type;
  int tier;
  int min;
  int max;
  double probDist;
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
  std::map<int, std::vector<RoomEdge>> adjacency;  // from map.json
  std::map<int, RoomConfig> rooms;                 // id -> per-room config
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

/**
 * @brief Parse a direction token ("N"/"E"/"S"/"W") from map.json.
 *
 * @throws std::runtime_error if the token isn't one of N/E/S/W.
 */
Direction parseDirection(const std::string& token);

/**
 * @brief Find the door on `room`'s edge matching `dir` (north at y=0, south
 * at y=HEIGHT-1, west at x=0, east at x=WIDTH-1).
 *
 * @throws std::runtime_error if `room` has no door on that edge.
 */
Coordinate resolveDoorForDirection(const Room& room, Direction dir);

#endif
