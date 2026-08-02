#ifndef DEBUG_LAYER_H
#define DEBUG_LAYER_H

#include "entities/player.h"
#include "render/render_stack.h"

class DebugLayer : public RenderStack {
 public:
  /**
   * @brief Construct a new Debug Layer.
   *
   * @param h Height of the layer window (in rows).
   * @param w Width of the layer window (in columns).
   * @param currentFps Current frames-per-second game is rendering at.
   * @param player The player object.
   */
  DebugLayer(int h, int w, const double& currentFps, const Player& player);

  /** @brief Draw frame timing and player position at the bottom row. */
  void doRender() override;

 private:
  const double& currentFps_;
  const Player& player_;
};

#endif