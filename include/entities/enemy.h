#ifndef ENEMY_H
#define ENEMY_H

#include <memory>
#include <optional>

#include "core/coordinate.h"
#include "entities/entity.h"
#include "entities/fov.h"

struct FrameState;
class GoalMapCache;
struct GameServices;

class Enemy : public Entity
{
 public:
  /**
   * @brief Construct a new Enemy object.
   *
   * @param position Starting position of enemy.
   * @param fov The enemy's sight radius, gating whether it can detect and
   * chase the player.
   * @param symbol Terminal representation of enemy. By default, a single
   * 'E' cell.
   * @param health Starting health of enemy. By default, 100.
   * @param speed Frames per move. For example, speed = 2, means an enemy can
   * move per every 2 frames/renders. By default, equal to 10.
   * @param attackDamage The attack damage of enemy. By default, 10.
   * @param chaseMemoryDuration Number of enemy moves the enemy will continue
   * hunting toward the last-seen player tile after losing line of sight. By
   * default, 5.
   */
  explicit Enemy(Coordinate position, std::unique_ptr<FOV> fov,
                 EntitySymbol symbol = {{'E'}}, int health = 100,
                 int speed = 10, int attackDamage = 10,
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
   * @param frame Per-frame world state (player and current room).
   * @param cache Goal-map cache used to path toward the chase target.
   * @param services RNG source used for movement tiebreaks and wandering.
   * @return bool True when the enemy attacks the player this frame (melee
   * currently always connects; this is the attack attempt, not a hit-chance
   * outcome).
   */
  bool moveTowardPlayer(const FrameState& frame, const GoalMapCache& cache,
                        GameServices& services);

 private:
  int attackDamage_;
  int chaseMemoryDuration_;
  int chaseTurnsRemaining_;
  std::optional<Coordinate> lastKnownPlayerPos_;

  // Frames remaining before the next attack attempt while blocked by the
  // player; decoupled from movement speed_ so attacking can run slower
  // than walking. Gates attack cadence, not whether an attack connects —
  // melee currently always hits, but this cooldown is the same one a future
  // ranged/projectile attack would use.
  int attackCooldownRemaining_;
};

#endif
