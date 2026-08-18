#include "render/layers/hud_layer.h"

#include <ncurses.h>

#include <algorithm>

#include "core/colors.h"
#include "render/ui.h"
#include "world/objects/weapon.h"

HUDLayer::HUDLayer(int h, int w, int margin, const Player& player,
                   const Level& graph)
    : RenderStack(h, w), player_(player), graph_(graph), margin_(margin)
{}

void HUDLayer::drawPlayerHealthBar(int row, int col)
{
  mvwprintw(win_, row, col, "HP:%d/%d", player_.getHealth(),
            player_.getMaxHealth());
};

void HUDLayer::drawRoomID(int row, int col)
{
  mvwprintw(win_, row, col, "Room:%d/%d", graph_.getCurrentRoomID() + 1,
            graph_.getRoomCount());
};

void HUDLayer::drawWeaponStats(int row, int col)
{
  const Weapon& weapon = player_.getWeapon();

  wattron(win_, colorAttr(weapon.getColor()));
  mvwprintw(win_, row, col, "%s DMG:%d SPD:%d RNG:%d", weapon.getName(),
            weapon.getDamage(), weapon.getSpeed(), weapon.getRange());
  wattroff(win_, colorAttr(weapon.getColor()));
};

void HUDLayer::doRender()
{
  werase(win_);  // need to erase each frame.

  // fixed HUD band above the map's top border, with a blank gap row
  // separating the HUD text from the border itself.
  UI geom = computeUI(height_, width_);
  int bandRow = std::max(0, geom.originY - margin_);

  // room number, in middle (subtract 4 to center better -- 4 chars in 'Room').
  this->drawRoomID(bandRow + 1, geom.originX + geom.winWidth / 2 - 4);

  // health bar & weapon stats top left.
  this->drawPlayerHealthBar(bandRow, geom.originX);
  this->drawWeaponStats(bandRow + 1, geom.originX);
};
