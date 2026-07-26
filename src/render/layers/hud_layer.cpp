#include "render/layers/hud_layer.h"

#include <ncurses.h>

#include <algorithm>

#include "core/colors.h"
#include "render/ui.h"
#include "world/weapon.h"

HUDLayer::HUDLayer(int h, int w, int margin, const Player& player,
                   const Level& level)
    : RenderStack(h, w), player(player), level(level), margin(margin) {}

void HUDLayer::drawPlayerHealthBar(int row, int col) {
  mvwprintw(win, row, col, "HP:%d/%d", player.getHealth(),
            player.getMaxHealth());
};

void HUDLayer::drawRoomID(int row, int col) {
  mvwprintw(win, row, col, "Room:%d/%d", level.getCurrentRoomID() + 1,
            level.getRoomCount());
};

void HUDLayer::drawWeaponStats(int row, int col) {
  const Weapon& weapon = player.getWeapon();

  wattron(win, colorAttr(weapon.getColor()));
  mvwprintw(win, row, col, "%s DMG:%d SPD:%d RNG:%d", weapon.getName(),
            weapon.getDamage(), weapon.getSpeed(), weapon.getRange());
  wattroff(win, colorAttr(weapon.getColor()));
};

void HUDLayer::doRender() {
  werase(win);  // need to erase each frame.

  // fixed HUD band above the map's top border, with a blank gap row
  // separating the HUD text from the border itself.
  UI geom = computeUI(height, width);
  int bandRow = std::max(0, geom.originY - margin);

  // room number, in middle (subtract 4 to center better -- 4 chars in 'Room').
  this->drawRoomID(bandRow + 1, geom.originX + geom.winWidth / 2 - 4);

  // health bar & weapon stats top left.
  this->drawPlayerHealthBar(bandRow, geom.originX);
  this->drawWeaponStats(bandRow + 1, geom.originX);
};
