#ifndef RENDERER_H
#define RENDERER_H

#include <map>
#include <memory>

#include "render/render_stack.h"

class Renderer
{
 public:
  /**
   * @brief Add a rendering layer at a given an index (z-order).
   *
   * @param z Z-order index.
   * @param layer RenderStack layer to add.
   */
  void addLayer(int z, std::unique_ptr<RenderStack> layer);

  /** @brief Composite all enabled layers onto stdscr and flush to terminal. */
  void compose();

  /**
   * @brief Resize all layers_ to a terminal resize.
   *
   * @param termHeight New terminal height (rows).
   * @param termWidth New terminal width (columns).
   */
  void resizeAll(int termHeight, int termWidth);

 private:
  std::map<int, std::unique_ptr<RenderStack>> layers_;
};

#endif
