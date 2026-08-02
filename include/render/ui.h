#ifndef UI_H
#define UI_H

#include <algorithm>

#include "world/map/room.h"

struct UI {
  int winHeight, winWidth;
  int originY, originX;
};

/**
 * @brief Compute the centered, terminal-clamped map window UI geometry.
 *
 * @param termHeight Current terminal height (rows).
 * @param termWidth  Current terminal width (columns).
 * @return UI
 */
inline UI computeUI(int termHeight, int termWidth) {
  UI g;
  g.winHeight = std::min(Room::HEIGHT, termHeight);
  g.winWidth = std::min(Room::WIDTH, termWidth);
  g.originY = std::max(0, (termHeight - Room::HEIGHT) / 2);
  g.originX = std::max(0, (termWidth - Room::WIDTH) / 2);
  return g;
}

#endif
