#ifndef COLORS_H
#define COLORS_H

#include <ncurses.h>

enum class ColorPair : short
{
  Default = 0,        ///< Terminal default (used inside the FoV).
  FogUnexplored = 1,  ///< Solid dark grey block over never-seen tiles.
  FogExplored = 2,    ///< Light grey terrain for previously seen tiles.

  EnemyDefault = 3,  ///< Placeholder for future enemy tinting.
  DoorDefault = 4,   ///< Placeholder for future door tinting.

  WeaponBasic = 5,   ///< Basic Bolt color.
  WeaponRapid = 6,   ///< Rapid Dart color.
  WeaponHeavy = 7,   ///< Heavy Slug color.
  WeaponSniper = 8,  ///< Sniper Round color.
};

/**
 * @brief Convert a ColorPair enum value into an ncurses attribute chtype.
 *
 * @param p Named colour pair.
 * @return chtype Attribute chunk suitable for bitwise-OR with a symbol.
 */
inline chtype colorAttr(ColorPair p)
{
  return COLOR_PAIR(static_cast<short>(p));
}

/** @brief Initialise ncurses colour support and register all named pairs. */
void initColors();

#endif
