#include "render/layers/entity_layer.h"

#include <ncurses.h>

#include <memory>

#include "render/ui.h"
#include "world/map/room.h"

EntityLayer::EntityLayer(
    int h, int w, int y, int x, const RoomGraph& graph, const Player& player,
    const std::vector<std::unique_ptr<Enemy>>& enemies,
    const std::vector<std::unique_ptr<Projectile>>& projectiles)
    : RenderStack(h, w, y, x),
      graph_(graph),
      player_(player),
      enemies_(enemies),
      projectiles_(projectiles) {}

void EntityLayer::drawEnemies() {
  const Room& room = graph_.getCurrentRoom();

  // iterate through vector of enemies.
  for (const auto& enemy : enemies_) {
    // if alive AND inside the player's current FoV, draw symbol.
    // Dynamic content (enemies) is never shown outside the FoV
    if (enemy->isAlive()) {
      Coordinate pos = enemy->getPosition();

      if (room.isVisible(pos.x, pos.y)) {
        // Hook: OR in colorAttr(ColorPair::EnemyDefault) — or a
        // per-enemy-type pair — once enemy colouring is designed.
        mvwaddch(win_, pos.y, pos.x, enemy->getSymbol());
      }
    };
  };
};

void EntityLayer::drawProjectiles() {
  const Room& room = graph_.getCurrentRoom();

  for (const auto& projectile : projectiles_) {
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

void EntityLayer::drawPlayer() {
  // if alive, draw symbol. Player is always at their own FoV origin so
  // no visibility check is needed here.
  if (player_.isAlive()) {
    Coordinate pos = player_.getPosition();

    mvwaddch(win_, pos.y, pos.x, player_.getSymbol());
  };
};

void EntityLayer::doRender() {
  werase(win_);  // need to erase each frame.

  // render enemies, then projectiles, then player (top of render).
  this->drawEnemies();
  this->drawProjectiles();
  this->drawPlayer();
};

void EntityLayer::onResize(int termHeight, int termWidth) {
  UI g = computeUI(termHeight, termWidth);
  reshape(g.winHeight, g.winWidth, g.originY, g.originX);
};
