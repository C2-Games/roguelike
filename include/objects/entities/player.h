#ifndef PLAYER_H
#define PLAYER_H

#include "objects/coordinate.h"
#include "objects/entities/entity.h"
#include "objects/weapons/weapon.h"

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
   * @param damage Damage to apply.
   */
  void takeDamage(Damage damage) override;

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

 private:
  int maxHealth_;
  Coordinate lastDirection_ = Coordinate(1, 0);
  Weapon weapon_;
};

#endif
