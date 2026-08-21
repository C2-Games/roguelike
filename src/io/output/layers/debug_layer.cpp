#include "io/output/layers/debug_layer.h"

#include <ncurses.h>

#include "io/output/render_state.h"

DebugLayer::DebugLayer(int h, int w) : RenderStack(h, w) {}

void DebugLayer::doRender(const RenderState& state)
{
  werase(win_);  // need to erase each frame.

  Coordinate pos = state.player.position;
  mvwprintw(win_, height_ - 1, 0, "FPS:%.2f|Position:(%d,%d)", state.fps, pos.x,
            pos.y);
};
