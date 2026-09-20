#ifndef PROJECTILE_SPAWN_H
#define PROJECTILE_SPAWN_H

#include <memory>

class Entity;
class Projectile;

namespace combat
{
/**
 * @brief Build a projectile fired from `entity`'s current position,
 * last-faced direction, and equipped weapon, gated by attack cooldown.
 *
 * @param entity Entity firing the projectile; mutated to reset the attack
 * cooldown on a successful shot.
 * @param fps Frames rendered per second, used to convert the weapon's fire
 * rate into a cooldown in frames.
 * @return The spawned projectile, or nullptr if the entity's attack cooldown
 * has not yet expired.
 */
std::unique_ptr<Projectile> spawnProjectile(Entity& entity, int fps);

/**
 * @brief Decrement the entity's remaining attack cooldown by one frame.
 *
 * @param entity Entity whose attack cooldown is ticked down.
 */
void tickAttackCooldown(Entity& entity);
}  // namespace combat

#endif
