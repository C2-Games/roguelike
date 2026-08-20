#include "render/layers/hud_layer.h"

#include <ncurses.h>

#include <algorithm>

#include "core/colors.h"
#include "render/window_position.h"
#include "world/objects/weapon.h"

namespace
{

// one bar cell per this much health, so the bar ticks in visible steps.
constexpr int HEALTH_INCREMENT = 5;

// fill colour changes at or below these absolute health values.
constexpr int HEALTH_STATUS_WARNING = 50;
constexpr int HEALTH_STATUS_CRITICAL = 20;

// U+2588 FULL BLOCK and U+2591 LIGHT SHADE.
constexpr wchar_t FILLED_CELL = L'█';
constexpr wchar_t EMPTY_CELL = L'░';

}  // namespace

HUDLayer::HUDLayer(int h, int w, int margin, const Player& player,
                   const Level& graph)
    : RenderStack(h, w), player_(player), graph_(graph), margin_(margin)
{}

void HUDLayer::drawBar(int row, int col, int filled, int total, ColorPair fill,
                       std::optional<ColorPair> empty)
{
  // block glyphs are not plain chars, so this needs the cchar_t path rather
  // than mvwaddch.
  wchar_t filledGlyph[] = {FILLED_CELL, L'\0'};
  cchar_t filledCell;
  setcchar(&filledCell, filledGlyph, A_NORMAL, static_cast<short>(fill),
           nullptr);

  cchar_t emptyCell;
  if (empty.has_value())
  {
    wchar_t emptyGlyph[] = {EMPTY_CELL, L'\0'};
    setcchar(&emptyCell, emptyGlyph, A_NORMAL, static_cast<short>(*empty),
             nullptr);
  }

  for (int cell = 0; cell < total; ++cell)
  {
    if (cell < filled)
    {
      mvwadd_wch(win_, row, col + cell, &filledCell);
    }
    else if (empty.has_value())
    {
      mvwadd_wch(win_, row, col + cell, &emptyCell);
    }
  }
}

void HUDLayer::drawPlayerHealthBar(int row, int col)
{
  const int health = player_.getHealth();
  const int maxHealth = player_.getMaxHealth();

  // round both up, so a max health that is not a multiple of HEALTH_INCREMENT
  // still fills completely at full health, and a sliver of health still shows
  // one cell rather than reading as dead.
  const int total = (maxHealth + HEALTH_INCREMENT - 1) / HEALTH_INCREMENT;
  const int filled =
      std::min(total, (health + HEALTH_INCREMENT - 1) / HEALTH_INCREMENT);

  ColorPair fill = ColorPair::HealthGood;
  if (health <= HEALTH_STATUS_CRITICAL)
  {
    fill = ColorPair::HealthCritical;
  }
  else if (health <= HEALTH_STATUS_WARNING)
  {
    fill = ColorPair::HealthWarn;
  }

  this->drawBar(row, col, filled, total, fill, ColorPair::BarEmpty);
  mvwprintw(win_, row, col + total + 1, "%3d/%d", health, maxHealth);
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
  WindowPosition geom = centerWindow(height_, width_);
  int bandRow = std::max(0, geom.originY - margin_);

  // room number, in middle (subtract 4 to center better -- 4 chars in 'Room').
  this->drawRoomID(bandRow + 1, geom.originX + geom.winWidth / 2 - 4);

  // health bar & weapon stats top left.
  this->drawPlayerHealthBar(bandRow, geom.originX);
  this->drawWeaponStats(bandRow + 1, geom.originX);
};
