#include "core/colors.h"

#include <ncurses.h>

namespace
{

// Extended 16-colour palette index 8 is "bright black" — dark grey on
// essentially every modern terminal.
constexpr short kGreyDark = 8;

// Small helper so pair registration reads naturally with the enum-class
// values
void registerPair(ColorPair id, short fg, short bg)
{
  init_pair(static_cast<short>(id), fg, bg);
}

}  // namespace

void initColors()
{
  if (!has_colors()) return;

  start_color();

  // Solid dark-grey block: fg == bg so the glyph itself is invisible and
  // only the cell background shows through. Any non-blank character in
  // MapLayer paired with this attribute renders as an opaque grey tile.
  registerPair(ColorPair::FogUnexplored, kGreyDark, kGreyDark);

  // Explored-but-not-visible: same grey background as FogUnexplored so
  // the "shadow" of previously seen area stays visible after the FoV
  // moves off it.
  registerPair(ColorPair::FogExplored, COLOR_WHITE, kGreyDark);

  // Weapon/projectile colors: one distinct foreground per WeaponType.
  registerPair(ColorPair::WeaponBasic, COLOR_CYAN, COLOR_BLACK);
  registerPair(ColorPair::WeaponRapid, COLOR_YELLOW, COLOR_BLACK);
  registerPair(ColorPair::WeaponHeavy, COLOR_RED, COLOR_BLACK);
  registerPair(ColorPair::WeaponSniper, COLOR_MAGENTA, COLOR_BLACK);

  // Health bar: one fill colour per threshold band, plus a dim grey for the
  // portion already lost so the bar's full width stays readable.
  registerPair(ColorPair::HealthGood, COLOR_GREEN, COLOR_BLACK);
  registerPair(ColorPair::HealthWarn, COLOR_YELLOW, COLOR_BLACK);
  registerPair(ColorPair::HealthCritical, COLOR_RED, COLOR_BLACK);
  registerPair(ColorPair::BarEmpty, kGreyDark, COLOR_BLACK);

  // Shield bar: blue fill for the overlay drawn on top of the health bar.
  registerPair(ColorPair::Shield, COLOR_BLUE, COLOR_BLACK);

  // One-shot flash overlay for the player symbol when an enemy hits them.
  registerPair(ColorPair::PlayerHit, COLOR_RED, COLOR_BLACK);
}
