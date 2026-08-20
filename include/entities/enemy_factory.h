#ifndef ENEMY_FACTORY_H
#define ENEMY_FACTORY_H

#include <memory>
#include <vector>

class Enemy;
struct Room;
struct GameServices;
struct EnemySpawnConfig;
class EnemyCatalog;

namespace enemy_factory
{

/**
 * @brief Roll a fresh set of enemies for a room from its spawn table.
 *
 * @param room       The room to populate; provides enemy spawn points.
 * @param spawnTable The room's authored enemy entries (name/tier/min/max/prob).
 * @param catalog    Resolves each entry's name/tier to stats.
 * @param services   RNG source.
 * @return Owning vector of newly-created enemies.
 */
std::vector<std::unique_ptr<Enemy>> rollForRoom(
    const Room& room, const std::vector<EnemySpawnConfig>& spawnTable,
    const EnemyCatalog& catalog, GameServices& services);

}  // namespace enemy_factory

#endif
