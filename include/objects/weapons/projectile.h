#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "objects/colors.h"
#include "objects/coordinate.h"
#include "objects/damage/damage.h"

struct Projectile
{
 public:
  /**
   * @brief Construct a new Projectile object.
   *
   * @param position Spawn tile -- the firing entity's own tile. Advancing
   * moves into the adjacent tile as its first candidate, so an entity
   * standing there is checked like any other tile in the projectile's path.
   * @param direction Unit step direction, e.g. (1,0)/(-1,0)/(0,1)/(0,-1).
   * @param damage Damage dealt to the first entity hit.
   * @param tilesPerTick Tiles advanced per Game::update() call.
   * @param range Max tiles traveled before the projectile expires.
   * @param color Color used to render the projectile's orb.
   */
  Projectile(Coordinate position, Coordinate direction, Damage damage,
             int tilesPerTick, int range, ColorPair color);

  /**
   * @brief Get the projectile's current position.
   *
   * @return Coordinate
   */
  Coordinate getPosition() const { return position_; }

  /**
   * @brief Get the projectile's direction of travel.
   *
   * @return Unit step offset, e.g. (1,0)/(-1,0)/(0,1)/(0,-1).
   */
  Coordinate getDirection() const { return direction_; }

  /**
   * @brief Get the projectile's render color.
   *
   * @return ColorPair
   */
  ColorPair getColor() const { return color_; }

  /**
   * @brief Get the damage this projectile deals on a hit.
   *
   * @return Damage dealt to whatever this projectile hits.
   */
  Damage getDamage() const { return damage_; }

  /**
   * @brief Get the number of tiles this projectile advances per tick.
   *
   * @return Tiles advanced per Game::update() call.
   */
  int getTilesPerTick() const { return tilesPerTick_; }

  /**
   * @brief Get the number of tiles remaining before this projectile expires.
   *
   * @return Tiles of range left before the projectile deactivates.
   */
  int getRemainingRange() const { return remainingRange_; }

  /**
   * @brief Whether the projectile is still in flight.
   *
   * @return bool
   */
  bool isActive() const { return active_; }

  /**
   * @brief Move the projectile to a new position.
   *
   * @param position Tile to move the projectile to.
   */
  void moveTo(Coordinate position) { position_ = position; }

  /** @brief Consume one tile of the projectile's remaining range. */
  void consumeRange() { --remainingRange_; }

  /** @brief Stop the projectile's flight. */
  void deactivate() { active_ = false; }

 private:
  Coordinate position_;
  Coordinate direction_;
  Damage damage_;
  int tilesPerTick_;
  int remainingRange_;
  ColorPair color_;
  bool active_ = true;
};

#endif
