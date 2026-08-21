#include "render/layers/entity_layer.h"

#include <ncurses.h>

#include <cstddef>
#include <memory>

#include "core/colors.h"
#include "entities/enemy.h"
#include "render/window_position.h"
#include "world/map/room.h"

namespace
{

// draws each non-empty cell of `symbol` at `origin + (col, row)`.
void drawSymbol(WINDOW* win, const EntitySymbol& symbol, Coordinate origin,
                const Room* room)
{
  for (std::size_t row = 0; row < symbol.size(); ++row)
  {
    for (std::size_t col = 0; col < symbol[row].size(); ++col)
    {
      const char cell = symbol[row][col];
      if (cell == '\0') continue;

      const int x = origin.x + static_cast<int>(col);
      const int y = origin.y + static_cast<int>(row);
      if (room != nullptr && !room->isVisible(x, y)) continue;

      mvwaddch(win, y, x, cell);
    }
  }
}

}  // namespace

EntityLayer::EntityLayer(
    int h, int w, int y, int x, const Level& graph, const Player& player,
    const std::vector<std::unique_ptr<Projectile>>& projectiles)
    : RenderStack(h, w, y, x),
      graph_(graph),
      player_(player),
      projectiles_(projectiles)
{}

void EntityLayer::drawEnemies()
{
  const Room& room = graph_.getCurrentRoom();

  // iterate through vector of enemies.
  for (const auto& enemy : room.enemies())
  {
    // if alive, draw the symbol cells that are inside the player's current
    // FoV. Dynamic content (enemies) is never shown outside the FoV.
    if (enemy->isAlive())
    {
      // Hook: OR in colorAttr(ColorPair::EnemyDefault) — or a
      // per-enemy pair — once enemy colouring is designed.
      drawSymbol(win_, enemy->getSymbol(), enemy->getPosition(), &room);
    };
  };
};

void EntityLayer::drawProjectiles()
{
  const Room& room = graph_.getCurrentRoom();

  for (const auto& projectile : projectiles_)
  {
    if (!projectile->isActive()) continue;

    Coordinate pos = projectile->getPosition();
    if (!room.isVisible(pos.x, pos.y)) continue;

    // wide-char draw: the orb is a true Unicode glyph (not a plain `char`),
    // so it needs cchar_t/setcchar/mvwadd_wch rather than mvwaddch.
    // Note: setcchar's colour argument is the raw ncurses pair number, not
    // a chtype from colorAttr().
    cchar_t cc;  // cchar_t = complex character type.
    wchar_t glyph[] = {L'●', L'\0'};
    short pairId = static_cast<short>(projectile->getColor());
    setcchar(&cc, glyph, A_NORMAL, pairId, nullptr);
    mvwadd_wch(win_, pos.y, pos.x, &cc);
  };
};

void EntityLayer::drawPlayer()
{
  // if alive, draw symbol. Player is always at their own FoV origin so
  // no visibility check is needed here.
  if (player_.isAlive())
  {
    const bool flashing = player_.isFlashing();
    if (flashing) wattron(win_, colorAttr(ColorPair::PlayerHit));
    drawSymbol(win_, player_.getSymbol(), player_.getPosition(), nullptr);
    if (flashing) wattroff(win_, colorAttr(ColorPair::PlayerHit));
  };
};

void EntityLayer::doRender()
{
  werase(win_);  // need to erase each frame.

  // render enemies, then projectiles, then player (top of render).
  this->drawEnemies();
  this->drawProjectiles();
  this->drawPlayer();
};

void EntityLayer::onResize(int termHeight, int termWidth)
{
  WindowPosition g = centerWindow(termHeight, termWidth);
  reshape(g.winHeight, g.winWidth, g.originY, g.originX);
};
