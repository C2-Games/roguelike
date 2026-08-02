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
 * @param room     The room to populate; provides tile grid for placement.
 * @param services RNG source.
 * @return Owning vector of newly-created enemies. Empty if the room has no
 *         floor tiles.
 */
std::vector<std::unique_ptr<Enemy>> rollForRoom(const Room& room,
                                                GameServices& services);

}  // namespace enemy_factory

#endif
