#ifndef ENEMY_H
#define ENEMY_H

#include <optional>

#include "core/coordinate.h"
#include "entities/entity.h"
#include "entities/fov.h"

struct MoveContext;

class Enemy : public Entity
{
 public:
  /**
   * @brief Construct a new Enemy object.
   *
   * @param x Starting column position of enemy.
   * @param y Starting row position of enemy.
   * @param symbol Terminal representation of enemy. By default, 'E'.
   * @param health Starting health of enemy. By default, 100.
   * @param speed Frames per move. For example, speed = 2, means an enemy can
   * move per every 2 frames/renders. By default, equal to 10.
   * @param attackDamage The attack damage of enemy. By default, 10.
   * @param attackFOV The FOV of the enemy. By default, an ellipse FOV w/
   * rx = 20, ry = 10.
   * @param chaseMemoryDuration Number of enemy moves the enemy will continue
   * hunting toward the last-seen player tile after losing line of sight. By
   * default, 5.
   */
  Enemy(int x, int y, char symbol = 'E', int health = 100, int speed = 10,
        int attackDamage = 10, FOV attackFOV = ellipseFOV(20, 10),
        int chaseMemoryDuration = 5);

  /**
   * @brief Get the enemy's attack damage.
   *
   * @return int
   */
  int getAttackDamage() const { return attackDamage_; };

  /**
   * @brief Reduce enemy health by damage amount.
   *
   * @param damage Amount of damage to apply.
   */
  void takeDamage(int damage) override;

  /**
   * @brief Advance enemy behavior one frame using wall-aware pathfinding.
   *
   * @param ctx Per-frame context (playerPos, current Room, goal-map cache,
   *   other enemies for collision, RNG source). Built once per frame by
   *   Game::update and reused across every enemy.
   */
  void moveTowardPlayer(const MoveContext& ctx);

 private:
  int attackDamage_;
  FOV attackFOV_;
  int chaseMemoryDuration_;
  int chaseTurnsRemaining_;
  std::optional<Coordinate> lastKnownPlayerPos_;
};

#endif
