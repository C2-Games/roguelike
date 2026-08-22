#include "io/output/layers/entity_layer.h"

#include <ncurses.h>

#include <cstddef>

#include "io/output/colors.h"
#include "io/output/render_state.h"
#include "io/output/window_position.h"
#include "world/map/room.h"

namespace
{

// draws each non-empty cell of `symbol` at `origin + (col, row)`. When
// `checkVisibility` is true, each cell is skipped unless it falls on a
// currently-visible tile of `state`.
void drawSymbol(WINDOW* win, const EntitySymbol& symbol, Coordinate origin,
                const RenderState& state, bool checkVisibility)
{
  for (std::size_t row = 0; row < symbol.size(); ++row)
  {
    for (std::size_t col = 0; col < symbol[row].size(); ++col)
    {
      const char cell = symbol[row][col];
      if (cell == '\0')
      {
        continue;
      }

      const int x = origin.x + static_cast<int>(col);
      const int y = origin.y + static_cast<int>(row);
      if (checkVisibility)
      {
        const bool outOfBounds =
            x < 0 || x >= Room::WIDTH || y < 0 || y >= Room::HEIGHT;
        if (outOfBounds ||
            state.tiles[x][y].visibility != TileVisibility::Visible)
        {
          continue;
        }
      }

      mvwaddch(win, y, x, cell);
    }
  }
}

}  // namespace

EntityLayer::EntityLayer(int h, int w, int y, int x) : RenderStack(h, w, y, x)
{}

void EntityLayer::drawEnemies(const RenderState& state)
{
  // Hook: OR in colorAttr(ColorPair::EnemyDefault) — or a
  // per-enemy pair — once enemy colouring is designed.
  for (const auto& enemy : state.enemies)
  {
    drawSymbol(win_, enemy.symbol, enemy.position, state, true);
  };
};

void EntityLayer::drawProjectiles(const RenderState& state)
{
  for (const auto& projectile : state.projectiles)
  {
    const Coordinate pos = projectile.position;
    const bool outOfBounds =
        pos.x < 0 || pos.x >= Room::WIDTH || pos.y < 0 || pos.y >= Room::HEIGHT;
    if (outOfBounds ||
        state.tiles[pos.x][pos.y].visibility != TileVisibility::Visible)
    {
      continue;
    }

    // wide-char draw: the orb is a true Unicode glyph (not a plain `char`),
    // so it needs cchar_t/setcchar/mvwadd_wch rather than mvwaddch.
    // Note: setcchar's colour argument is the raw ncurses pair number, not
    // a chtype from colorAttr().
    cchar_t cc;  // cchar_t = complex character type.
    wchar_t glyph[] = {L'●', L'\0'};
    short pairId = static_cast<short>(projectile.color);
    setcchar(&cc, glyph, A_NORMAL, pairId, nullptr);
    mvwadd_wch(win_, pos.y, pos.x, &cc);
  };
};

void EntityLayer::drawPlayer(const RenderState& state)
{
  if (state.player.tinted)
  {
    wattron(win_, colorAttr(state.player.tintColor));
  }
  drawSymbol(win_, state.player.symbol, state.player.position, state, false);
  if (state.player.tinted)
  {
    wattroff(win_, colorAttr(state.player.tintColor));
  }
};

void EntityLayer::doRender(const RenderState& state)
{
  werase(win_);  // need to erase each frame.

  // render enemies, then projectiles, then player (top of render).
  this->drawEnemies(state);
  this->drawProjectiles(state);
  this->drawPlayer(state);
};

void EntityLayer::onResize(int termHeight, int termWidth)
{
  WindowPosition g = centerWindow(termHeight, termWidth);
  reshape(g.winHeight, g.winWidth, g.originY, g.originX);
};
