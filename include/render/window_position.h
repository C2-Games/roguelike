#ifndef WINDOW_POSITION_H
#define WINDOW_POSITION_H

#include <algorithm>

#include "world/map/room.h"

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
  g.winHeight = std::min(Room::HEIGHT, termHeight);
  g.winWidth = std::min(Room::WIDTH, termWidth);
  g.originY = std::max(0, (termHeight - Room::HEIGHT) / 2);
  g.originX = std::max(0, (termWidth - Room::WIDTH) / 2);
  return g;
}

#endif
