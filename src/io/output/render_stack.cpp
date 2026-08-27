#include "io/output/render_stack.h"

RenderStack::RenderStack(int h, int w, int y, int x)
    : win_(newwin(h, w, y, x)), height_(h), width_(w), originY_(y), originX_(x)
{}

void RenderStack::reshape(int h, int w, int y, int x)
{
  if (win_ != nullptr)
  {
    delwin(win_);
  }
  win_ = newwin(h, w, y, x);
  height_ = h;
  width_ = w;
  originY_ = y;
  originX_ = x;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void RenderStack::addWideGlyph(int y, int x, wchar_t glyph, ColorPair pair)
{
  cchar_t cc;  // cchar_t = complex character type.
  wchar_t cells[] = {glyph, L'\0'};
  if (setcchar(&cc, cells, A_NORMAL, static_cast<short>(pair), nullptr) != OK)
  {
    return;
  }
  mvwadd_wch(win_, y, x, &cc);
}
