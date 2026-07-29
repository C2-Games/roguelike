#ifndef PROJECTILE_CONTEXT_H
#define PROJECTILE_CONTEXT_H

#include <functional>

#include "core/coordinate.h"

struct Room;

/**
 * @brief Per-frame context passed to Projectile::update.
 *
 * Decouples Projectile from any specific entity storage layout: instead of
 * scanning an enemy vector directly, the projectile asks the ctx to try a
 * hit at a coordinate. The closure returned by Game encapsulates whatever
 * "damage the entity at this position" means today (currently: the enemy
 * vector), and can grow later without touching Projectile at all.
 */
struct ProjectileContext {
  const Room& room;  ///< For walkability / bounds tests.

  /**
   * @brief Try to deal `damage` at world-tile `pos`.
   *
   * Returns true if damage was applied (projectile should deactivate),
   * false if the tile is empty of damageable entities.
   */
  std::function<bool(Coordinate pos, int damage)> tryHit;
};

#endif
