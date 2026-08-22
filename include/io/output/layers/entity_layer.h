#ifndef ENTITY_LAYER_H
#define ENTITY_LAYER_H

#include "io/output/render_stack.h"

struct EntityLayerPacket;

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
   * @brief Draw enemy entities.
   *
   * @param data This layer's per-frame render data.
   */
  void drawEnemies(const EntityLayerPacket& data);

  /**
   * @brief Draw active projectiles.
   *
   * @param data This layer's per-frame render data.
   */
  void drawProjectiles(const EntityLayerPacket& data);

  /**
   * @brief Draw player entity.
   *
   * @param data This layer's per-frame render data.
   */
  void drawPlayer(const EntityLayerPacket& data);

  /**
   * @brief Draw all alive entities into the layer window.
   *
   * @param data This layer's per-frame render data.
   */
  void doRender(const EntityLayerPacket& data);

  /**
   * @brief Recompute the centered, terminal-clamped map window geometry.
   *
   * @param termHeight New terminal height (rows).
   * @param termWidth New terminal width (columns).
   */
  void onResize(int termHeight, int termWidth) override;
};

#endif
