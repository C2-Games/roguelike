#ifndef DEBUG_LAYER_H
#define DEBUG_LAYER_H

#include "io/output/render_stack.h"

struct DebugLayerPacket;

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
   * @param data This layer's per-frame render data.
   */
  void doRender(const DebugLayerPacket& data);
};

#endif
