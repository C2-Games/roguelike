#include "io/output/renderer.h"

#include <ncurses.h>

void Renderer::addLayer(RenderLayer z, std::unique_ptr<RenderStack> layer)
{
  layers_[z] = std::move(layer);
};

void Renderer::compose(const RenderState& state)
{
  erase();  // erase stdscr first.

  // iterate through the "layers_", update them, render, and then layer them.
  for (auto& [z, layer] : layers_)
  {
    if (layer->isEnabled())
    {
      layer->doUpdate();
      layer->doRender(state);

      // overlay the ncurses::WINDOW object.
      overlay(layer->getWindow(), stdscr);
    };
  };

  // flush to terminal.
  refresh();
};

void Renderer::resizeAll(int termHeight, int termWidth)
{
  for (auto& [z, layer] : layers_)
  {
    layer->onResize(termHeight, termWidth);
  };
};
