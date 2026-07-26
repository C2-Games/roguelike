#ifndef PLAYER_H
#define PLAYER_H

#include "core/coordinate.h"
#include "entities/entity.h"
#include "world/fov.h"
#include "world/weapon.h"

class Player : public Entity {
 public:
  /**
   * @brief Construct a new Player object.
   *
   * @param x Starting column position of player.
   * @param y Starting row position of player.
   * @param health Starting health of player.
   * @param speed Frames per move. For example, speed = 2, means a player can
   * move per every 2 frames/renders. By default, equal to 1.
   * @param sightRx Horizontal FoV radius (columns). By default, 16.
   * @param sightRy Vertical FoV radius (rows). By default, 10. Vertical is
   * smaller because a row of terminal cells is taller than a column is wide.
   */
  Player(int x, int y, int health = 100, int speed = 1, int sightRx = 16,
         int sightRy = 10);

  /**
   * @brief Get the initial max health of player.
   *
   * @return int
   */
  int getMaxHealth() const { return maxHealth; }

  /**
   * @brief Reduce player health by damage amount.
   *
   * @param damage Amount of damage to apply.
   */
  void takeDamage(int damage) override;

  /**
   * @brief Get the player's current field-of-view mask.
   *
   * The FOV is cached and only rebuilt when setSightRadius() is called.
   *
   * @return const FOV& Offset set centred on the player.
   */
  const FOV& getFOV() const { return fov; }

  /**
   * @brief Get the horizontal sight radius (columns).
   *
   * @return int
   */
  int getSightRadiusX() const { return sightRx; }

  /**
   * @brief Get the vertical sight radius (rows).
   *
   * @return int
   */
  int getSightRadiusY() const { return sightRy; }

  /**
   * @brief Change the player's sight radii and rebuild the cached FOV.
   *
   * Intended for future sight-modifying items (torches, curses, potions).
   *
   * @param rx New horizontal FoV radius (columns).
   * @param ry New vertical FoV radius (rows).
   */
  void setSightRadius(int rx, int ry);

  /**
   * @brief Get the direction the player last faced (from movement input).
   *
   * @return Coordinate
   */
  Coordinate getLastDirection() const { return lastDirection; }

  /**
   * @brief Set the direction the player last faced.
   *
   * @param dir New facing direction.
   */
  void setLastDirection(Coordinate dir) { lastDirection = dir; }

  /**
   * @brief Get the player's currently equipped weapon.
   *
   * @return const Weapon&
   */
  const Weapon& getWeapon() const { return weapon; }

 private:
  int maxHealth;
  int sightRx;
  int sightRy;
  FOV fov;
  Coordinate lastDirection = Coordinate(1, 0);
  Weapon weapon;
};

#endif
