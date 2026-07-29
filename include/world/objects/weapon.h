#ifndef WEAPON_H
#define WEAPON_H

#include <unordered_map>

#include "core/colors.h"

enum class WeaponType { Basic, Rapid, Heavy, Sniper };

struct WeaponAttributes {
  int damage;
  int speed;  // tiles traveled per Game::update() tick
  int range;  // max tiles traveled before the projectile expires
  ColorPair color;
  const char* name;
};

class Weapon {
 public:
  /**
   * @brief Construct a new Weapon object.
   *
   * @param type The weapon type. By default, WeaponType::Basic.
   */
  explicit Weapon(WeaponType type = WeaponType::Basic);

  /**
   * @brief Get the weapon type.
   *
   * @return WeaponType
   */
  WeaponType getType() const { return type; }

  /**
   * @brief Get the weapon's projectile damage.
   *
   * @return int
   */
  int getDamage() const;

  /**
   * @brief Get the weapon's projectile travel speed (tiles per tick).
   *
   * @return int
   */
  int getSpeed() const;

  /**
   * @brief Get the weapon's projectile max range (tiles).
   *
   * @return int
   */
  int getRange() const;

  /**
   * @brief Get the weapon's color, used for both the HUD stat line and the
   * projectile's rendered orb.
   *
   * @return ColorPair
   */
  ColorPair getColor() const;

  /**
   * @brief Get the weapon's display name.
   *
   * @return const char*
   */
  const char* getName() const;

 private:
  WeaponType type;

  // lookup table for weapon attributes based on WeaponType.
  static const std::unordered_map<int, WeaponAttributes> typeAttributes;
};

#endif
