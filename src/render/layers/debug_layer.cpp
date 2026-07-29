#include "render/layers/debug_layer.h"

DebugLayer::DebugLayer(int h, int w, const double& currentFps,
                       const Player& player)
    : RenderStack(h, w), currentFps_(currentFps), player_(player) {}

void DebugLayer::doRender() {
  werase(win_);  // need to erase each frame.

  Coordinate pos = player_.getPosition();
  mvwprintw(win_, height_ - 1, 0, "FPS:%.2f|Position:(%d,%d)", currentFps_,
            pos.x, pos.y);
};
