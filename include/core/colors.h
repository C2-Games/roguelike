#ifndef COLORS_H
#define COLORS_H

#include <ncurses.h>

/**
 * @brief Named ncurses colour-pair identifiers used across render layers.
 *
 * Pair 0 is reserved by ncurses for the terminal default and cannot be
 * overridden. All other pairs are initialised in initColors().
 */
enum class ColorPair : short {
  Default = 0,        ///< Terminal default (used inside the FoV).
  FogUnexplored = 1,  ///< Solid dark grey block over never-seen tiles.
  FogExplored = 2,    ///< Light grey terrain for previously seen tiles.

  // -- Reserved hooks (not yet initialised) ------------------------------
  EnemyDefault = 3,  ///< Placeholder for future enemy tinting.
  DoorDefault = 4,   ///< Placeholder for future door tinting.

  // -- Weapon/projectile colors ------------------------------------------
  WeaponBasic = 5,   ///< Basic Bolt color.
  WeaponRapid = 6,   ///< Rapid Dart color.
  WeaponHeavy = 7,   ///< Heavy Slug color.
  WeaponSniper = 8,  ///< Sniper Round color.
};

/**
 * @brief Convert a ColorPair enum value into an ncurses attribute chtype.
 *
 * Wraps COLOR_PAIR() so callers can write
 * `mvwaddch(win, y, x, sym | colorAttr(ColorPair::FogExplored))` without
 * sprinkling casts.
 *
 * @param p Named colour pair.
 * @return chtype Attribute chunk suitable for bitwise-OR with a symbol.
 */
inline chtype colorAttr(ColorPair p) {
  return COLOR_PAIR(static_cast<short>(p));
}

/**
 * @brief Initialise ncurses colour support and register all named pairs.
 *
 * Must be called after initscr() and before any window draws. On terminals
 * that do not support color the call is a no-op — every layer will fall
 * back to attribute-only rendering.
 */
void initColors();

#endif
