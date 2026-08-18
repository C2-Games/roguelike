#include "render/render_stack.h"

RenderStack::RenderStack(int h, int w, int y, int x)
    : win_(newwin(h, w, y, x)), height_(h), width_(w), originY_(y), originX_(x)
{}

void RenderStack::reshape(int h, int w, int y, int x)
{
  if (win_) delwin(win_);
  win_ = newwin(h, w, y, x);
  height_ = h;
  width_ = w;
  originY_ = y;
  originX_ = x;
}
