#ifndef LOADER_H
#define LOADER_H

#include <filesystem>

class Level;
struct GameServices;

namespace loader
{

/**
 * @brief Load a level directory (level.json, map.json, and every room's
 * JSON metadata + referenced .txt template), wire the room graph, spawn
 * every room's enemies, and seal any doors left unlinked by this level's
 * adjacency back to Wall tiles.
 *
 * @param levelDir Directory containing the level's config files.
 * @param assetsDir Root assets directory (e.g. "assets"), used to locate
 *                  the enemy catalog and room templates.
 * @param services Shared services; used transiently to roll each room's
 *                 enemy spawns.
 * @return A fully-built Level.
 * @throws std::runtime_error if the enemy catalog or any level file is
 *         missing/malformed.
 */
Level loadLevel(const std::filesystem::path& levelDir,
                const std::filesystem::path& assetsDir, GameServices& services);

}  // namespace loader

#endif
