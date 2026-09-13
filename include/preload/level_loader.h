#ifndef LEVEL_LOADER_H
#define LEVEL_LOADER_H

#include <filesystem>

struct LevelData;
struct GameServices;

namespace preload
{

/**
 * @brief Load a level's rooms, room graph, and enemy spawns from the
 * database, wire the room graph, spawn every room's enemies, and seal any
 * doors left unlinked by this level's adjacency back to Wall tiles.
 *
 * @param levelID The level's id in the levels table.
 * @param dbPath Path to the SQLite database.
 * @param assetsDir Root assets directory (e.g. "assets"), used to locate
 *                  room templates.
 * @param services Shared services; used transiently to roll each room's
 *                 enemy spawns.
 * @return A fully-built LevelData.
 * @throws std::runtime_error if levelID has no matching row in the levels
 *         table.
 * @throws SQLite::Exception if dbPath can't be opened or a query fails.
 */
LevelData loadLevel(int levelID, const std::filesystem::path& dbPath,
                    const std::filesystem::path& assetsDir,
                    GameServices& services);

}  // namespace preload

#endif
