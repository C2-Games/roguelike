#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "core/colors.h"
#include "core/coordinate.h"

struct ProjectileContext;

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
   * the first wall collision, on the first successful hit reported by
   * `ctx.tryHit`, or once its range is exhausted.
   *
   * @param ctx Per-frame context (current Room for walkability + a hit-sink
   *   closure the projectile calls when it lands on a potentially-damaged
   *   coordinate). Built once per frame by Game::update.
   */
  void update(const ProjectileContext& ctx);

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
