#ifndef RENDERER_H
#define RENDERER_H

#include <cstdint>
#include <map>
#include <memory>

#include "io/output/render_stack.h"

struct RenderState;

// composition order, lowest first. the values are the z-order itself, so
// changing one reorders the screen.
enum class RenderLayer : std::uint8_t
{
  Map = 1,
  Entity = 2,
  HUD = 3,
  Debug = 4
};

class Renderer
{
 public:
  /**
   * @brief Add a rendering layer at a given an index (z-order).
   *
   * @param z Z-order index.
   * @param layer RenderStack layer to add.
   */
  void addLayer(RenderLayer z, std::unique_ptr<RenderStack> layer);

  /**
   * @brief Composite all enabled layers onto stdscr and flush to terminal.
   *
   * @param state Per-frame render snapshot passed to every layer.
   */
  void compose(const RenderState& state);

  /**
   * @brief Resize all layers_ to a terminal resize.
   *
   * @param termHeight New terminal height (rows).
   * @param termWidth New terminal width (columns).
   */
  void resizeAll(int termHeight, int termWidth);

 private:
  std::map<RenderLayer, std::unique_ptr<RenderStack>> layers_;
};

#endif
