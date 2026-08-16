#ifndef ENEMY_FACTORY_H
#define ENEMY_FACTORY_H

#include <memory>
#include <vector>

class Enemy;
struct Room;
struct GameServices;

namespace enemy_factory {

/**
 * @brief Roll a fresh set of enemies for a room.
 *
 * Each of the room's authored enemy spawn points independently has a 50%
 * chance of producing an enemy, alternating placeholder type (goblin/ogre)
 * by spawn-list order.
 *
 * @param room     The room to populate; provides enemy spawn points.
 * @param services RNG source.
 * @return Owning vector of newly-created enemies. Empty if the room has no
 *         enemy spawn points.
 */
std::vector<std::unique_ptr<Enemy>> rollForRoom(const Room& room,
                                                GameServices& services);

}  // namespace enemy_factory

#endif
