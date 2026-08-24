#ifndef WINDOW_POSITION_H
#define WINDOW_POSITION_H

#include <algorithm>

#include "objects/room/room_dimensions.h"

struct WindowPosition
{
  int winHeight, winWidth;
  int originY, originX;
};

/**
 * @brief Compute the centered, terminal-clamped map window UI geometry.
 *
 * @param termHeight Current terminal height (rows).
 * @param termWidth  Current terminal width (columns).
 * @return WindowPosition
 */
inline WindowPosition centerWindow(int termHeight, int termWidth)
{
  WindowPosition g;
  g.winHeight = std::min(ROOM_HEIGHT, termHeight);
  g.winWidth = std::min(ROOM_WIDTH, termWidth);
  g.originY = std::max(0, (termHeight - ROOM_HEIGHT) / 2);
  g.originX = std::max(0, (termWidth - ROOM_WIDTH) / 2);
  return g;
}

#endif
