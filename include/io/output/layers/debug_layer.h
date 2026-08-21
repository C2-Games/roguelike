#ifndef DEBUG_LAYER_H
#define DEBUG_LAYER_H

#include "io/output/render_stack.h"

struct RenderState;

class DebugLayer : public RenderStack
{
 public:
  /**
   * @brief Construct a new Debug Layer.
   *
   * @param h Height of the layer window (in rows).
   * @param w Width of the layer window (in columns).
   */
  DebugLayer(int h, int w);

  /**
   * @brief Draw frame timing and player position at the bottom row.
   *
   * @param state Per-frame render snapshot to draw from.
   */
  void doRender(const RenderState& state) override;
};

#endif
