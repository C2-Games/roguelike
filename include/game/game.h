#ifndef GAME_H
#define GAME_H

#include <chrono>
#include <cstdint>
#include <memory>
#include <vector>

#include "game/level.h"
#include "game/services.h"
#include "io/output/render_state.h"
#include "objects/entities/player.h"
#include "objects/fovs/ellipse_fov.h"
#include "objects/fovs/fov.h"
#include "objects/weapons/projectile.h"
#include "systems/combat/combat.h"
#include "systems/loader/loader.h"
#include "systems/movement/movement.h"
#include "systems/visibility/visibility.h"

class UIManager;

enum class GameState : std::uint8_t
{
  Play,
  Start,
  Pause,
  End,
  TransLevel
};

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

  /**
   * @brief Get the game's current state.
   *
   * @return The game's current phase.
   */
  GameState getState() const { return state_; };

  /**
   * @brief Set the game's current state.
   *
   * @param state The state to transition to.
   */
  void setState(GameState state) { state_ = state; };

 private:
  const int fps_;
  double currentFps_ = 0.0;
  GameServices services_;
  Player player_;
  Level level_;
  GoalMapCache goalMapCache_;
  std::vector<std::unique_ptr<Projectile>> projectiles_;

  GameState state_ = GameState::Play;
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
   * @brief Snapshot the live game object graph into a stateless RenderState
   * for the io/output/ render layers to consume.
   *
   * @return A render-ready snapshot of the current frame's world state.
   */
  RenderState buildRenderState() const;

  /**
   * @brief Renders the game state. Draws the player, enemies, and UI elements
   * on the screen.
   */
  void render();
};

#endif
