#ifndef ENTITY_LAYER_H
#define ENTITY_LAYER_H

#include <memory>
#include <vector>

#include "entities/enemy.h"
#include "entities/player.h"
#include "render/render_stack.h"
#include "world/level.h"
#include "world/projectile.h"

class EntityLayer : public RenderStack {
 public:
  /**
   * @brief Construct a new Entity Layer.
   *
   * NOTE: The reason we want to pass the player & enemies seperately is because
   * we want to render the player last. Such that if a player attacks an enemy
   * via melee, the player will render on top of the enemy.
   *
   * @param h Height of the layer window (in rows).
   * @param w Width of the layer window (in columns).
   * @param y Row of the window's top-left corner in the terminal.
   * @param x Column of the window's top-left corner in the terminal.
   * @param level The level, used to query per-tile FoV visibility so
   * entities outside the player's sight are not drawn.
   * @param player The player entity to draw each frame.
   * @param enemies The enemy list to draw each frame.
   * @param projectiles The projectile list to draw each frame.
   */
  EntityLayer(int h, int w, int y, int x, const Level& level,
              const Player& player,
              const std::vector<std::unique_ptr<Enemy>>& enemies,
              const std::vector<std::unique_ptr<Projectile>>& projectiles);

  /**
   * @brief Draw enemy entities that are inside the player's current FoV.
   *
   */
  void drawEnemies();

  /**
   * @brief Draw active projectiles that are inside the player's current FoV.
   *
   */
  void drawProjectiles();

  /**
   * @brief Draw player entity.
   *
   */
  void drawPlayer();

  /**
   * @brief Draw all alive entities into the layer window.
   *
   */
  void doRender() override;

  /**
   * @brief Recompute the centered, terminal-clamped map window geometry.
   *
   * @param termHeight New terminal height (rows).
   * @param termWidth New terminal width (columns).
   */
  void onResize(int termHeight, int termWidth) override;

 private:
  const Level& level;
  const Player& player;
  const std::vector<std::unique_ptr<Enemy>>& enemies;
  const std::vector<std::unique_ptr<Projectile>>& projectiles;
};

#endif