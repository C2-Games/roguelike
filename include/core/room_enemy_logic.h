#ifndef ROOM_ENEMY_LOGIC_H
#define ROOM_ENEMY_LOGIC_H

#include <memory>
#include <vector>

class Enemy;
struct Room;
struct GameServices;
struct EnemySpawnConfig;
class EnemyCatalog;

// stopgap home for RoomEnemyState logic that reaches into Enemy -- belongs in
// systems/ once that layer exists.
namespace room_enemy_logic
{

/**
 * @brief Roll `room`'s enemies on first call; a no-op on every call after.
 *
 * @param room       The room to populate; its RoomEnemyState is filled in.
 * @param spawnTable The room's authored enemy entries.
 * @param catalog    Resolves each entry's name/tier to stats.
 * @param services   RNG source for the factory roll.
 */
void ensureSpawned(Room& room, const std::vector<EnemySpawnConfig>& spawnTable,
                   const EnemyCatalog& catalog, GameServices& services);

/** @brief Drop dead enemies from `active`. */
void reap(std::vector<std::unique_ptr<Enemy>>& active);

}  // namespace room_enemy_logic

#endif
