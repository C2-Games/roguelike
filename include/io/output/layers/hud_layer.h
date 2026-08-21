#ifndef HUD_LAYER_H
#define HUD_LAYER_H

#include <optional>

#include "core/colors.h"
#include "io/output/render_stack.h"

struct RenderState;

class HUDLayer : public RenderStack
{
 public:
  /**
   * @brief Construct a new HUD Layer.
   *
   * @param h      Height of the layer window (in rows).
   * @param w      Width of the layer window in columns.
   * @param margin The margin between HUD & map layer.
   */
  HUDLayer(int h, int w, const int margin);

  /**
   * @brief Draw the player's health as a block-glyph bar, tinted by how much
   * health remains, with the numeric value alongside it.
   *
   * @param state Per-frame render snapshot to draw from.
   * @param row Absolute row to draw the health bar at.
   * @param col Absolute column to draw the health bar at.
   */
  void drawPlayerHealthBar(const RenderState& state, int row, int col);

  /**
   * @brief Draw the current room ID and total room count at a fixed screen
   * position.
   *
   * @param state Per-frame render snapshot to draw from.
   * @param row Absolute row to draw the room indicator at.
   * @param col Absolute column to draw the room indicator at.
   */
  void drawRoomID(const RenderState& state, int row, int col);

  /**
   * @brief Draw the player's current weapon name and stats at a fixed
   * screen position, tinted with the weapon's colour.
   *
   * @param state Per-frame render snapshot to draw from.
   * @param row Absolute row to draw the weapon stats at.
   * @param col Absolute column to draw the weapon stats at.
   */
  void drawWeaponStats(const RenderState& state, int row, int col);

  /**
   * @brief Draw the HP indicator, room ID, and weapon stats each frame.
   *
   * @param state Per-frame render snapshot to draw from.
   */
  void doRender(const RenderState& state) override;

 private:
  /**
   * @brief Draw one horizontal bar of block glyphs. The filled cells take
   * `fill`; the remainder is drawn dim when an empty colour is supplied, so
   * the bar's full width stays visible.
   *
   * @param row Absolute row to draw at.
   * @param col Absolute column the bar starts at.
   * @param filled Number of cells to draw as filled.
   * @param total Total cells in the bar.
   * @param fill Colour pair for the filled portion.
   * @param empty Colour pair for the unfilled remainder, or nullopt to leave
   * those cells untouched
   */
  void drawBar(int row, int col, int filled, int total, ColorPair fill,
               std::optional<ColorPair> empty);

  const int margin_;
};

#endif
