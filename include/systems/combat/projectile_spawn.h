#ifndef PROJECTILE_SPAWN_H
#define PROJECTILE_SPAWN_H

#include <memory>

#include "objects/entities/player.h"

class Projectile;

namespace combat
{
/**
 * @brief Build a projectile fired from `player`'s current position,
 * last-faced direction, and equipped weapon, gated by attack cooldown.
 *
 * @param player Player firing the projectile; mutated to reset the attack
 * cooldown on a successful shot.
 * @param fps Frames rendered per second, used to convert the weapon's fire
 * rate into a cooldown in frames.
 * @return The spawned projectile, or nullptr if the player's attack cooldown
 * has not yet expired.
 */
std::unique_ptr<Projectile> spawnProjectile(Player& player, int fps);

/**
 * @brief Decrement the player's remaining attack cooldown by one frame.
 *
 * @param player Player whose attack cooldown is ticked down.
 */
void tickAttackCooldown(Player& player);
}  // namespace combat

#endif
