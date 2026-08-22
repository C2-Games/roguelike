#ifndef ROOM_ENEMY_STATE_H
#define ROOM_ENEMY_STATE_H

#include <memory>
#include <vector>

#include "core/coordinate.h"
#include "entities/enemy.h"
#include "world/map/room_types.h"

struct Room;
struct GameServices;
class EnemyCatalog;

/**
 * @brief A room's enemy state: whether it has been rolled yet, and the live
 * enemies currently in it.
 */
class RoomEnemyState
{
 public:
  RoomEnemyState() = default;

  /**
   * @brief Roll this room's enemies on first call; a no-op on every call
   * after.
   *
   * @param room       The owning room, for spawn-point placement.
   * @param spawnTable The room's authored enemy entries.
   * @param catalog    Resolves each entry's name/tier to stats.
   * @param services   RNG source for the factory roll.
   */
  void ensureSpawned(const Room& room,
                     const std::vector<EnemySpawnConfig>& spawnTable,
                     const EnemyCatalog& catalog, GameServices& services);

  std::vector<std::unique_ptr<Enemy>>& enemies() { return enemies_; }
  const std::vector<std::unique_ptr<Enemy>>& enemies() const
  {
    return enemies_;
  }

  /**
   * @brief Find the live enemy standing on `pos`.
   *
   * @param pos Tile to test.
   * @param exclude Enemy skipped by pointer identity, so a caller asking
   * about its own tile never matches itself. Null considers every enemy.
   * @return The enemy on `pos`, or nullptr when none is there.
   */
  Enemy* enemyAt(Coordinate pos, const Enemy* exclude = nullptr) const;

  /** @brief Drop dead enemies from `active`. */
  static void reap(std::vector<std::unique_ptr<Enemy>>& active);

 private:
  bool spawned_ = false;
  std::vector<std::unique_ptr<Enemy>> enemies_;
};

#endif
