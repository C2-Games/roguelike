#ifndef ENEMY_H
#define ENEMY_H

#include <cstdint>
#include <memory>
#include <optional>

#include "objects/coordinate.h"
#include "objects/entities/entity.h"
#include "objects/fovs/fov.h"

enum class AIState : std::uint8_t
{
  Sentry,  // stationary/patrol default: never spotted the player, or gave up
           // searching.
  Chase,   // player is currently in FoV; target is their live position.
  Search,  // lost sight; target is the last-known position.
};

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
   * @brief Get the enemy's current AI state.
   *
   * @return The enemy's current AI state.
   */
  AIState getAIState() const { return aiState_; }

  /**
   * @brief Set the enemy's AI state.
   *
   * @param state New AI state.
   */
  void setAIState(AIState state) { aiState_ = state; }

  /**
   * @brief Get the number of moves remaining in chase memory.
   *
   * @return The number of moves remaining before chase memory expires.
   */
  int getChaseTurnsRemaining() const { return chaseTurnsRemaining_; }

  /**
   * @brief Set the number of moves remaining in chase memory.
   *
   * @param turns New number of moves remaining.
   */
  void setChaseTurnsRemaining(int turns) { chaseTurnsRemaining_ = turns; }

  /**
   * @brief Get the last known player position.
   *
   * @return The last known player position, or std::nullopt when none is
   * remembered.
   */
  std::optional<Coordinate> getLastKnownPlayerPos() const
  {
    return lastKnownPlayerPos_;
  }

  /**
   * @brief Set the last known player position.
   *
   * @param pos New last-known player position, or std::nullopt to clear it.
   */
  void setLastKnownPlayerPos(std::optional<Coordinate> pos)
  {
    lastKnownPlayerPos_ = pos;
  }

  /**
   * @brief Get the number of moves the enemy remembers the player's last
   * position for after losing line of sight.
   *
   * @return The chase memory duration.
   */
  int getChaseMemoryDuration() const { return chaseMemoryDuration_; }

  /**
   * @brief Get the number of frames remaining before the next attack
   * attempt.
   *
   * @return The number of frames remaining.
   */
  int getAttackCooldownRemaining() const { return attackCooldownRemaining_; }

  /**
   * @brief Set the number of frames remaining before the next attack
   * attempt.
   *
   * @param frames New number of frames remaining.
   */
  void setAttackCooldownRemaining(int frames)
  {
    attackCooldownRemaining_ = frames;
  }

 private:
  int attackDamage_;
  int chaseMemoryDuration_;
  int chaseTurnsRemaining_;
  std::optional<Coordinate> lastKnownPlayerPos_;
  AIState aiState_;

  // frames remaining before the next attack attempt
  int attackCooldownRemaining_;
};

#endif
