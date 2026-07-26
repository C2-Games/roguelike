#ifndef HUD_LAYER_H
#define HUD_LAYER_H

#include "entities/player.h"
#include "render/render_stack.h"
#include "world/level.h"

class HUDLayer : public RenderStack {
 public:
  /**
   * @brief Construct a new HUD Layer.
   *
   * @param h      Height of the layer window (in rows).
   * @param w      Width of the layer window in columns.
   * @param margin The margin between HUD & map layer.
   * @param player The player object to track stats, health, position, etc.
   * @param level  The level to read the current room ID from.
   */
  HUDLayer(int h, int w, const int margin, const Player& player,
           const Level& level);

  /**
   * @brief Draw player health bar at a fixed screen position.
   *
   * @param row Absolute row to draw the health bar at.
   * @param col Absolute column to draw the health bar at.
   */
  void drawPlayerHealthBar(int row, int col);

  /**
   * @brief Draw the current room ID and total room count at a fixed screen
   * position.
   *
   * @param row Absolute row to draw the room indicator at.
   * @param col Absolute column to draw the room indicator at.
   */
  void drawRoomID(int row, int col);

  /**
   * @brief Draw the player's current weapon name and stats at a fixed
   * screen position, tinted with the weapon's colour.
   *
   * @param row Absolute row to draw the weapon stats at.
   * @param col Absolute column to draw the weapon stats at.
   */
  void drawWeaponStats(int row, int col);

  /**
   * @brief Draw the HP indicator, room ID, and weapon stats each frame.
   *
   */
  void doRender() override;

 private:
  const Player& player;
  const Level& level;
  const int margin;
};

#endif