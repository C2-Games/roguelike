#ifndef ENEMY_H
#define ENEMY_H

#include <memory>
#include <optional>
#include <vector>

#include "core/coordinate.h"
#include "entities/entity.h"
#include "entities/fov.h"

class Level;

class Enemy : public Entity {
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
  int getAttackDamage() const { return attackDamage; };

  /**
   * @brief Reduce enemy health by damage amount.
   *
   * @param damage Amount of damage to apply.
   */
  void takeDamage(int damage) override;

  /**
   * @brief Advance enemy behavior one frame using wall-aware pathfinding.
   *
   * Behavior selection (in order):
   * 1. If the player is inside the enemy's attackFOV: refresh chase memory
   *    (lastKnownPlayerPos = playerPos, chaseTurnsRemaining reset) and step
   *    down-gradient toward the player via Level::getGoalMap.
   * 2. Else if chase memory is active: step down-gradient toward the
   *    remembered tile. On each actual move, decrement chaseTurnsRemaining;
   *    on arrival or expiry, clear the memory.
   * 3. Else: wander randomly to a walkable Floor neighbor.
   *
   * Ties in the goal map are broken randomly. If the chosen neighbor is
   * occupied by another live enemy, the enemy falls back to the next-lowest
   * reachable neighbor before giving up. Position updates are throttled by
   * Entity::speed via Entity::moveTo.
   *
   * @param playerPos  Current player world position.
   * @param level      Level reference; supplies goal maps and current-room
   *                   tile lookups.
   * @param allEnemies Every enemy in the current room. Used for
   *                   enemy-vs-enemy collision detection during step
   *                   selection and wander.
   */
  void moveTowardPlayer(Coordinate playerPos, Level& level,
                        const std::vector<std::unique_ptr<Enemy>>& allEnemies);

 private:
  int attackDamage;
  FOV attackFOV;

  /// Turns (actual moves) the enemy will keep hunting toward the last-seen
  /// player tile after losing line of sight. Set at construction, immutable
  /// afterward.
  int chaseMemoryDuration;

  /// Remaining hunting-memory moves. Decrements by one per actual move
  /// while the player is out of FoV. Reset to chaseMemoryDuration whenever
  /// the player re-enters FoV.
  int chaseTurnsRemaining;

  /// Last tile the player was seen on. Empty when the enemy has never
  /// spotted the player or when chase memory has expired.
  std::optional<Coordinate> lastKnownPlayerPos;
};

#endif
