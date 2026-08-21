#ifndef PLAYER_H
#define PLAYER_H

#include "core/coordinate.h"
#include "entities/entity.h"
#include "world/objects/weapon.h"

class Player : public Entity
{
 public:
  /**
   * @brief Construct a new Player object.
   *
   * @param position Starting position of player.
   * @param health Starting health of player.
   * @param speed Frames per move. For example, speed = 2, means a player can
   * move per every 2 frames/renders. By default, equal to 1.
   */
  explicit Player(Coordinate position, int health = 100, int speed = 1);

  /**
   * @brief Get the initial max health of player.
   *
   * @return int
   */
  int getMaxHealth() const { return maxHealth_; }

  /**
   * @brief Reduce player health by damage amount.
   *
   * @param damage Amount of damage to apply.
   */
  void takeDamage(int damage) override;

  /**
   * @brief Rebuild the player's cached FoV mask from new radii.
   *
   * @param rx New horizontal FoV radius (columns).
   * @param ry New vertical FoV radius (rows).
   */
  void changeFOV(int rx, int ry);

  /**
   * @brief Get the direction the player last faced (from movement input).
   *
   * @return Coordinate
   */
  Coordinate getLastDirection() const { return lastDirection_; }

  /**
   * @brief Set the direction the player last faced.
   *
   * @param dir New facing direction.
   */
  void setLastDirection(Coordinate dir) { lastDirection_ = dir; }

  /**
   * @brief Get the player's currently equipped weapon.
   *
   * @return const Weapon&
   */
  const Weapon& getWeapon() const { return weapon_; }

  /** @brief Start (or restart) the one-shot red hit-flash. */
  void triggerHitFlash() { hitFlashFramesRemaining_ = HIT_FLASH_FRAMES; }

  /** @brief Advance the hit-flash timer by one frame. */
  void tickHitFlash()
  {
    if (hitFlashFramesRemaining_ > 0) --hitFlashFramesRemaining_;
  }

  /** @brief Whether the one-shot hit-flash is currently active. */
  bool isFlashing() const { return hitFlashFramesRemaining_ > 0; }

 private:
  // Duration of the one-shot red hit-flash, in frames.
  static constexpr int HIT_FLASH_FRAMES = 8;

  int maxHealth_;
  Coordinate lastDirection_ = Coordinate(1, 0);
  Weapon weapon_;
  int hitFlashFramesRemaining_ = 0;
};

#endif
