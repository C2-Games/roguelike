#ifndef ENTITY_LAYER_H
#define ENTITY_LAYER_H

#include "io/output/render_stack.h"

struct RenderState;

class EntityLayer : public RenderStack
{
 public:
  /**
   * @brief Construct a new Entity Layer.
   *
   * @param h Height of the layer window (in rows).
   * @param w Width of the layer window (in columns).
   * @param y Row of the window's top-left corner in the terminal.
   * @param x Column of the window's top-left corner in the terminal.
   */
  EntityLayer(int h, int w, int y, int x);

  /**
   * @brief Draw enemy entities that are inside the player's current FoV.
   *
   * @param state Per-frame render snapshot to draw from.
   */
  void drawEnemies(const RenderState& state);

  /**
   * @brief Draw active projectiles that are inside the player's current FoV.
   *
   * @param state Per-frame render snapshot to draw from.
   */
  void drawProjectiles(const RenderState& state);

  /**
   * @brief Draw player entity.
   *
   * @param state Per-frame render snapshot to draw from.
   */
  void drawPlayer(const RenderState& state);

  /**
   * @brief Draw all alive entities into the layer window.
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
