#ifndef PLAYER_H
#define PLAYER_H

#include "objects/coordinate.h"
#include "objects/entities/entity.h"

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
   * @brief Rebuild the player's cached FoV mask from new radii.
   *
   * @param rx New horizontal FoV radius (columns).
   * @param ry New vertical FoV radius (rows).
   */
  void changeFOV(int rx, int ry);

 private:
  int maxHealth_;
};

#endif
