#ifndef RENDER_STACK_H
#define RENDER_STACK_H

#include <ncurses.h>

struct RenderState;

class RenderStack
{
 public:
  /**
   * @brief Construct a new Render Stack object.
   *
   * @param h Height of the layer window (in rows).
   * @param w Width of the layer window (in columns).
   * @param y Row of the window's top-left corner in the terminal. By
   * default, 0.
   * @param x Column of the window's top-left corner in the terminal. By
   * default, 0.
   */
  RenderStack(int h, int w, int y = 0, int x = 0);

  // need to make sure the ncurses::WINDOW is deleted if object is.
  virtual ~RenderStack()
  {
    if (win_ != nullptr)
    {
      delwin(win_);
    }
  };

  // doing this to enforce no copies. each RenderStack object should be unique.
  RenderStack(const RenderStack&) = delete;
  RenderStack& operator=(const RenderStack&) = delete;
  RenderStack(RenderStack&&) = delete;
  RenderStack& operator=(RenderStack&&) = delete;

  /** @brief State update hook. Default is a no-op. */
  virtual void doUpdate() {};

  /**
   * @brief Terminal-resize hook. Default recreates the window to span the
   * full terminal at origin (0,0).
   *
   * @param termHeight New terminal height (rows).
   * @param termWidth New terminal width (columns).
   */
  virtual void onResize(int termHeight, int termWidth)
  {
    reshape(termHeight, termWidth, 0, 0);
  }

  /**
   * @brief Draw this layer's content on the WINDOW.
   *
   * @param state Per-frame render snapshot to draw from.
   */
  virtual void doRender(const RenderState& state) = 0;

  /**
   * @brief Enable or disable this layer. Disabled layers are skipped entirely
   * by Renderer::compose().
   *
   * @param e True to enable, false to disable.
   */
  void setEnabled(bool e) { enabled_ = e; };

  /**
   * @brief Check whether this layer is currently enabled.
   *
   * @return bool
   */
  bool isEnabled() const { return enabled_; };

  /**
   * @brief Get the WINDOW.
   *
   * @return WINDOW*
   */
  WINDOW* getWindow() const { return win_; };

 protected:
  /**
   * @brief Recreate the window with a new size/origin.
   *
   * @param h New height (in rows).
   * @param w New width (in columns).
   * @param y New row of the window's top-left corner. By default, 0.
   * @param x New column of the window's top-left corner. By default, 0.
   */
  void reshape(int h, int w, int y = 0, int x = 0);

  WINDOW* win_;
  int height_;
  int width_;
  int originY_;
  int originX_;

 private:
  bool enabled_ = true;
};

#endif
