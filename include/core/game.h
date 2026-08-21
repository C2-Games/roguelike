#ifndef GAME_H
#define GAME_H

#include <chrono>
#include <memory>
#include <vector>

#include "core/services.h"
#include "entities/ellipse_fov.h"
#include "entities/enemy_catalog.h"
#include "entities/fov.h"
#include "entities/player.h"
#include "render/layers/debug_layer.h"
#include "render/layers/entity_layer.h"
#include "render/layers/hud_layer.h"
#include "render/layers/map_layer.h"
#include "render/renderer.h"
#include "world/map/level.h"
#include "world/objects/projectile.h"
#include "world/systems/goal_map_cache.h"

class Game
{
 public:
  /**
   * @brief Construct a new Game:: Game object.
   *
   * @param width Current terminal width (columns).
   * @param height Current terminal height (rows).
   * @param fps frames per second of game. By default, 60.
   *
   */
  Game(int width, int height, int fps = 60);

  /** @brief Runs the main game loop. */
  void run();

 private:
  int termWidth_, termHeight_;
  const int fps_;
  double currentFps_;
  GameServices services_;
  Player player_;
  EnemyCatalog enemyCatalog_;
  Level level_;
  GoalMapCache goalMapCache_;
  std::vector<std::unique_ptr<Projectile>> projectiles_;

  bool isRunning_;
  Renderer renderer_;
  // Non-owning; owned by renderer_. Its hit-flash timer only ticks while
  // this layer stays enabled (RenderStack::doUpdate() is skipped otherwise).
  EntityLayer* entityLayer_ = nullptr;

  Coordinate lastVisibilityPos_ = Coordinate(-1, -1);
  int lastVisibilityRoomID_ = -1;
  std::unique_ptr<FOV> lastVisibilityFov_ =
      std::make_unique<EllipseFOV>(-1, -1);

  /**
   * @brief Get the frame duraction in milliseconds.
   *
   * @return std::chrono::milliseconds
   */
  auto getDuration() const { return std::chrono::milliseconds(1000 / fps_); };

  /** @brief Handles user input for player movement and game controls. */
  void handleInput();

  /**
   * @brief Updates the game state - moves enemies toward the player and checks
   * for collisions.
   */
  void update();

  /**
   * @brief Whether the player has moved or changed rooms since visibility was
   * last recomputed.
   *
   * @return True when the field of view is stale and needs recomputing.
   */
  bool playerMoved() const;

  /**
   * @brief Renders the game state. Draws the player, enemies, and UI elements
   * on the screen.
   */
  void render();
};

#endif
