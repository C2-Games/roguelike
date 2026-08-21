#ifndef MAP_LAYER_H
#define MAP_LAYER_H

#include "core/colors.h"
#include "io/output/render_stack.h"

struct RenderState;

class MapLayer : public RenderStack
{
 public:
  /**
   * @brief Construct a new Map Layer.
   *
   * @param h Height of the layer window (in rows).
   * @param w Width of the layer window (in columns).
   * @param y Row of the window's top-left corner in the terminal.
   * @param x Column of the window's top-left corner in the terminal.
   */
  MapLayer(int h, int w, int y, int x);

  /**
   * @brief Draw all room tiles into the map layer window.
   *
   * @param state Per-frame render snapshot to draw from.
   */
  void drawMap(const RenderState& state);

  /**
   * @brief Render map layer window.
   *
   * @param state Per-frame render snapshot to draw from.
   */
  void doRender(const RenderState& state) override;

  /**
   * @brief Recompute the centered, terminal-clamped map window geometry.
   *
   * @param termHeight New terminal height (rows).
   * @param termWidth New terminal width (columns).
   */
  void onResize(int termHeight, int termWidth) override;
};

#endif
