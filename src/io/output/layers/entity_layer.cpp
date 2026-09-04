#include "io/output/layers/entity_layer.h"

#include <ncurses.h>

#include <cstddef>

#include "io/output/colors.h"
#include "io/output/render_state.h"
#include "io/output/window_position.h"

namespace
{

// draws each non-empty cell of `symbol` at `origin + (col, row)`.
void drawSymbol(WINDOW* win, const EntitySymbol& symbol, Coordinate origin)
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
      mvwaddch(win, y, x, cell);
    }
  }
}

}  // namespace

EntityLayer::EntityLayer(int h, int w, int y, int x) : RenderStack(h, w, y, x)
{}

void EntityLayer::drawEnemies(const EntityLayerPacket& data)
{
  // Hook: OR in colorAttr(ColorPair::EnemyDefault) — or a
  // per-enemy pair — once enemy colouring is designed.
  for (const auto& enemy : data.enemies)
  {
    drawSymbol(win_, enemy.symbol, enemy.position);
  };
};

void EntityLayer::drawProjectiles(const EntityLayerPacket& data)
{
  for (const auto& projectile : data.projectiles)
  {
    const Coordinate pos = projectile.position;

    // the orb is a true Unicode glyph, not a plain char.
    addWideGlyph(pos.y, pos.x, L'●', projectile.color);
  };
};

void EntityLayer::drawPlayer(const EntityLayerPacket& data)
{
  if (data.player.tinted)
  {
    wattron(win_, colorAttr(data.player.tintColor));
  }
  drawSymbol(win_, data.player.symbol, data.player.position);
  if (data.player.tinted)
  {
    wattroff(win_, colorAttr(data.player.tintColor));
  }
};

void EntityLayer::doRender(const EntityLayerPacket& data)
{
  werase(win_);  // need to erase each frame.

  // render enemies, then projectiles, then player (top of render).
  this->drawEnemies(data);
  this->drawProjectiles(data);
  this->drawPlayer(data);
};

void EntityLayer::onResize(int termHeight, int termWidth)
{
  WindowPosition g = centerWindow(termHeight, termWidth);
  reshape(g.winHeight, g.winWidth, g.originY, g.originX);
};
