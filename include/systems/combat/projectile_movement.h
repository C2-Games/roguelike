#ifndef PROJECTILE_MOVEMENT_H
#define PROJECTILE_MOVEMENT_H

#include <memory>
#include <vector>

class Enemy;
class Player;
class Projectile;
struct Room;

namespace combat
{
// advance a projectile up to its tiles-per-tick, deactivating it on a wall
// collision, on landing on a live entity (applying its damage), or once its
// range is exhausted.
void advanceProjectile(Projectile& projectile, const Room& room,
                       const std::vector<std::unique_ptr<Enemy>>& enemies,
                       Player& player);
}  // namespace combat

#endif
