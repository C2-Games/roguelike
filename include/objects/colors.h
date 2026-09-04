#ifndef COLORS_H
#define COLORS_H

#include <cstdint>

enum class ColorPair : std::uint8_t
{
  Default = 0,        ///< Terminal default (used inside the FoV).
  FogUnexplored = 1,  ///< Solid dark grey block over never-seen tiles.
  FogExplored = 2,    ///< Light grey terrain for previously seen tiles.

  EnemyDefault = 3,  ///< Placeholder for future enemy tinting.
  DoorDefault = 4,   ///< Placeholder for future door tinting.

  WeaponBasic = 5,   ///< Basic Bolt color.
  WeaponRapid = 6,   ///< Rapid Dart color.
  WeaponHeavy = 7,   ///< Heavy Slug color.
  WeaponSniper = 8,  ///< Sniper Round color.

  HealthGood = 9,       ///< Health bar fill above the warning threshold.
  HealthWarn = 10,      ///< Health bar fill at or below 50 health.
  HealthCritical = 11,  ///< Health bar fill at or below 20 health.
  BarEmpty = 12,        ///< Unfilled remainder of any bar.
  Shield = 13,          ///< Reserved for the shield overlay bar.

  EntityHit = 14,  ///< One-shot red flash on any entity (player or enemy) that
                   ///< just took damage.
};

#endif
