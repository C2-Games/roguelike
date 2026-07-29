#ifndef MOVE_CONTEXT_H
#define MOVE_CONTEXT_H

#include <memory>
#include <vector>

#include "core/coordinate.h"

class Enemy;
class GoalMapCache;
struct GameServices;
struct Room;

/**
 * @brief Per-turn context passed to Enemy::moveTowardPlayer.
 *
 * Bundles everything an enemy needs to decide and execute one movement
 * frame. Constructed once per frame by Game::update and passed by
 * const-reference to every enemy in the room — the enemy never holds a
 * long-lived reference to any world subsystem, only whatever it captures
 * from this ctx during the call.
 *
 * All references point to objects owned by Game and are guaranteed to
 * outlive the enemy call itself.
 */
struct MoveContext {
  Coordinate playerPos;  ///< Player's current tile.
  const Room& room;      ///< Room the enemy is currently in.
  GoalMapCache& cache;   ///< For pathfinding lookups (in-FoV + chase memory).
  const std::vector<std::unique_ptr<Enemy>>&
      allEnemies;  ///< For enemy-vs-enemy collision.
  GameServices&
      services;  ///< RNG source (Fisher-Yates tie-break, wander pick).
};

#endif
