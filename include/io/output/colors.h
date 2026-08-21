#ifndef IO_OUTPUT_COLORS_H
#define IO_OUTPUT_COLORS_H

#include <ncurses.h>

#include "core/colors.h"

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
