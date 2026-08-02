#ifndef PROJECTILE_CONTEXT_H
#define PROJECTILE_CONTEXT_H

#include <functional>

#include "core/coordinate.h"

struct Room;

struct ProjectileContext {
  const Room& room;

  /** @brief Try to deal `damage` at world-tile `pos`. */
  std::function<bool(Coordinate pos, int damage)> tryHit;
};

#endif
