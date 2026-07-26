#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <memory>
#include <vector>

#include "core/colors.h"
#include "core/coordinate.h"
#include "entities/enemy.h"
#include "world/room.h"

class Projectile {
 public:
  /**
   * @brief Construct a new Projectile object.
   *
   * @param position Spawn tile, already offset one step from the firing
   * entity in the facing direction.
   * @param direction Unit step direction, e.g. (1,0)/(-1,0)/(0,1)/(0,-1).
   * @param damage Damage dealt to the first enemy hit.
   * @param tilesPerTick Tiles advanced per Game::update() call.
   * @param range Max tiles traveled before the projectile expires.
   * @param color Color used to render the projectile's orb.
   */
  Projectile(Coordinate position, Coordinate direction, int damage,
             int tilesPerTick, int range, ColorPair color);

  /**
   * @brief Advance the projectile up to tilesPerTick tiles. Deactivates on
   * the first collision or once its range is exhausted.
   *
   * @param room Current room, for bounds/walkability checks.
   * @param enemies Enemy list; a hit enemy takes damage via takeDamage().
   */
  void update(const Room& room, std::vector<std::unique_ptr<Enemy>>& enemies);

  /**
   * @brief Get the projectile's current position.
   *
   * @return Coordinate
   */
  Coordinate getPosition() const { return position; }

  /**
   * @brief Get the projectile's render color.
   *
   * @return ColorPair
   */
  ColorPair getColor() const { return color; }

  /**
   * @brief Whether the projectile is still in flight.
   *
   * @return bool
   */
  bool isActive() const { return active; }

 private:
  Coordinate position;
  Coordinate direction;
  int damage;
  int tilesPerTick;
  int remainingRange;
  ColorPair color;
  bool active = true;

  void deactivate() { active = false; }
};

#endif
