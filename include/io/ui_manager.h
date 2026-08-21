#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "io/input/game_commands.h"
#include "io/output/renderer.h"

struct RenderState;

class UIManager
{
 public:
  /** @brief Initialise ncurses and build the render layer stack. */
  UIManager();

  /** @brief End ncurses mode, if not already ended. */
  ~UIManager();

  // owns ncurses' global state and a Renderer of unique_ptrs; not copyable
  // or movable.
  UIManager(const UIManager&) = delete;
  UIManager& operator=(const UIManager&) = delete;
  UIManager(UIManager&&) = delete;
  UIManager& operator=(UIManager&&) = delete;

  /**
   * @brief Poll ncurses for the next input command.
   *
   * @return The mapped GameCommand. A terminal resize is resolved internally
   * (resizes the render layers) and reported back as GameCommand::None.
   */
  GameCommand pollInput();

  /**
   * @brief Push a render snapshot through the owned layer stack.
   *
   * @param state Per-frame render snapshot to draw.
   */
  void render(const RenderState& state);

  /** @brief Show the "press SPACE to begin" intro screen and block for it. */
  static void showStartScreen();

  /** @brief End ncurses mode and print the game-over message. */
  static void showGameOver();

 private:
  int termWidth_, termHeight_;
  Renderer renderer_;
};

#endif
