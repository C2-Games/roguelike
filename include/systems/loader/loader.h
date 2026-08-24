#ifndef LOADER_H
#define LOADER_H

#include <filesystem>

#include "systems/loader/enemy_catalog.h"

class Level;
struct GameServices;
struct Room;

class Loader
{
 public:
  /**
   * @brief Load and parse every enemy definition under `assetsDir/enemies`.
   *
   * @param assetsDir Root assets directory (e.g. "assets").
   * @throws std::runtime_error if the enemy catalog directory is
   *         missing/empty or a file fails to parse.
   */
  explicit Loader(const std::filesystem::path& assetsDir);

  /**
   * @brief Load a level directory (level.json, map.json, and every room's
   * JSON metadata + referenced .txt template), wire the room graph, spawn
   * every room's enemies, and seal any doors left unlinked by this level's
   * adjacency back to Wall tiles.
   *
   * @param levelDir Directory containing the level's config files.
   * @param services Shared services; used transiently to roll each room's
   *                 enemy spawns.
   * @return A fully-built Level.
   */
  Level loadLevel(const std::filesystem::path& levelDir,
                  GameServices& services) const;

 private:
  std::filesystem::path assetsDir_;
  EnemyCatalog catalog_;

  std::filesystem::path enemyCatalogDir() const
  {
    return assetsDir_ / "enemies";
  }

  std::filesystem::path roomsDir() const { return assetsDir_ / "rooms"; }

  // load a Room from a text file authored under assets/rooms/, given the
  // unique roomID to assign it and the path to its file.
  Room loadRoom(int roomID, const std::filesystem::path& path) const;
};

#endif
