#ifndef PROJECTILE_SPAWN_H
#define PROJECTILE_SPAWN_H

#include <memory>

class Player;
class Projectile;

namespace combat
{
// build a projectile fired from `player`'s current position, last-faced
// direction, and equipped weapon.
std::unique_ptr<Projectile> spawnProjectile(const Player& player);
}  // namespace combat

#endif
