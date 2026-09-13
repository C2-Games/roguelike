#include "io/output/layers/entity_layer.h"

#include <ncurses.h>

#include <cstddef>

#include "io/output/render_state.h"
#include "io/output/window_position.h"

EntityLayer::EntityLayer(int h, int w, int y, int x) : RenderStack(h, w, y, x)
{}

void EntityLayer::drawSymbol(const EntitySymbol& symbol, Coordinate origin,
                             ColorPair color)
{
  for (std::size_t row = 0; row < symbol.size(); ++row)
  {
    for (std::size_t col = 0; col < symbol[row].size(); ++col)
    {
      const wchar_t cell = symbol[row][col];
      if (cell == L'\0')
      {
        continue;
      }

      const int x = origin.x + static_cast<int>(col);
      const int y = origin.y + static_cast<int>(row);
      addWideGlyph(y, x, cell, color);
    }
  }
}

void EntityLayer::drawEnemies(const EntityLayerPacket& data)
{
  // per-enemy hit-flash tint is honoured below; the untinted enemy still draws
  // in the terminal default. hook: pass ColorPair::EnemyDefault for that base
  // case once enemy colouring is designed.
  for (const auto& enemy : data.enemies)
  {
    const ColorPair color = enemy.tinted ? enemy.tintColor : ColorPair::Default;
    drawSymbol(enemy.symbol, enemy.position, color);
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
  const ColorPair color =
      data.player.tinted ? data.player.tintColor : ColorPair::Default;
  drawSymbol(data.player.symbol, data.player.position, color);
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
