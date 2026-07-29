#ifndef FORMAT_TOKENS_H
#define FORMAT_TOKENS_H

/**
 * @brief Format tokens used by the `assets/rooms/*.txt` parser.
 *
 * These are the characters and keywords the parser recognizes when loading
 * a Room from disk. They also appear (as documentation) in the .txt files
 * themselves, so keep the two in sync when adding new tile types or header
 * keys.
 */
namespace format_tokens {

// ---- Header syntax ----
/// Prefix that marks a header line ("@name: foo").
inline constexpr char kHeaderPrefix = '@';
/// Separator between header key and value ("name: foo").
inline constexpr char kHeaderSeparator = ':';
/// Header key: human-readable name of the room (currently stored on Room).
inline constexpr const char* kHeaderKeyName = "name";
/// Header key: which levels the room is eligible for (parsed by the room
/// library layer; currently unused by loadFromFile).
inline constexpr const char* kHeaderKeyLevels = "levels";

// ---- Tile-char legend (grid body) ----
inline constexpr char kTileWall = '#';
inline constexpr char kTileFloor = '.';
inline constexpr char kTilePillar = 'o';
inline constexpr char kTileVoid = ' ';

// ---- Spawn markers ----
// Each spawn marker resolves to a Floor tile in the grid and additionally
// records its coordinate on the matching Room::*Spawns vector for downstream
// systems (enemy_factory, ItemRegistry, ...) to consume.
inline constexpr char kSpawnEnemy = '!';
inline constexpr char kSpawnLoot = '$';
inline constexpr char kSpawnItem = '?';

}  // namespace format_tokens

#endif
