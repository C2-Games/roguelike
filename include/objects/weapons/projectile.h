#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "objects/colors.h"
#include "objects/coordinate.h"
#include "objects/damage/damage.h"
#include "objects/direction.h"

struct Projectile
{
 public:
  /**
   * @brief Construct a new Projectile object.
   *
   * @param position Spawn tile -- the firing entity's own tile. Advancing
   * moves into the adjacent tile as its first candidate, so an entity
   * standing there is checked like any other tile in the projectile's path.
   * @param direction Direction the projectile travels in.
   * @param damage Damage dealt to the first entity hit.
   * @param tilesPerTick Tiles advanced per Game::update() call.
   * @param range Max tiles traveled before the projectile expires.
   * @param ammoSymbol Glyph copied from the firing weapon's ammo at spawn
   * time.
   * @param crit Combined firing-entity + weapon crit-chance tiers
   * (2x/3x/4x/5x/10x).
   */
  Projectile(Coordinate position, Direction direction, Damage damage,
             int tilesPerTick, int range, char ammoSymbol,
             const double (&crit)[5]);

  /**
   * @brief Get the projectile's current position.
   *
   * @return Coordinate
   */
  Coordinate getPosition() const { return position_; }

  /**
   * @brief Get the projectile's direction of travel.
   *
   * @return The projectile's direction of travel.
   */
  Direction getDirection() const { return direction_; }

  /**
   * @brief Get the projectile's render color.
   *
   * @return ColorPair
   */
  ColorPair getColor() const { return color_; }

  /**
   * @brief Get this projectile's ammo glyph.
   *
   * @return The ammo glyph copied from the firing weapon at spawn time.
   */
  char getAmmoSymbol() const { return ammoSymbol_; }

  /**
   * @brief Get this projectile's combined crit-chance tiers.
   *
   * @return const double(&)[5]
   */
  const double (&getCrit() const)[5] { return crit_; }

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
  Direction direction_;
  Damage damage_;
  int tilesPerTick_;
  int remainingRange_;
  ColorPair color_;
  char ammoSymbol_;
  double crit_[5];
  bool active_ = true;
};

#endif
