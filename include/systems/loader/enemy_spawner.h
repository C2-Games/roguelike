#ifndef ENEMY_SPAWNER_H
#define ENEMY_SPAWNER_H

#include <vector>

#include "objects/room/room_types.h"

struct Room;
struct GameServices;
class EnemyCatalog;

namespace enemy_spawner
{

// roll room's enemies on first call, filling in its enemiesSpawned/enemies
// fields from spawnTable (resolved against catalog); a no-op on every call
// after. services is the RNG source for the roll.
void ensureSpawned(Room& room, const std::vector<EnemySpawnConfig>& spawnTable,
                   const EnemyCatalog& catalog, GameServices& services);

}  // namespace enemy_spawner

#endif
