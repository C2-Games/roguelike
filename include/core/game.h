#ifndef GAME_H
#define GAME_H

#include <chrono>
#include <memory>
#include <vector>

#include "core/services.h"
#include "entities/ellipse_fov.h"
#include "entities/fov.h"
#include "entities/player.h"
#include "preload/enemy_catalog.h"
#include "world/map/level.h"
#include "world/objects/projectile.h"
#include "world/systems/goal_map_cache.h"

class UIManager;

class Game
{
 public:
  /**
   * @brief Construct a new Game:: Game object.
   *
   * @param uiManager The I/O boundary Game reads input from and pushes
   * render snapshots through. Must outlive this Game.
   * @param fps frames per second of game. By default, 60.
   *
   */
  explicit Game(UIManager& uiManager, int fps = 60);

  /** @brief Runs the main game loop. */
  void run();

 private:
  const int fps_;
  double currentFps_ = 0.0;
  GameServices services_;
  Player player_;
  EnemyCatalog enemyCatalog_;
  Level level_;
  GoalMapCache goalMapCache_;
  std::vector<std::unique_ptr<Projectile>> projectiles_;

  bool isRunning_;
  UIManager& uiManager_;
  int playerHitFlashFramesRemaining_ = 0;

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
