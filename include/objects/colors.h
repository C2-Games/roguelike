#ifndef COLORS_H
#define COLORS_H

#include <cstdint>

#include "objects/damage/damage_type.h"

enum class ColorPair : std::uint8_t
{
  Default = 0,        ///< Terminal default (used inside the FoV).
  FogUnexplored = 1,  ///< Solid dark grey block over never-seen tiles.
  FogExplored = 2,    ///< Light grey terrain for previously seen tiles.

  EnemyDefault = 3,  ///< Placeholder for future enemy tinting.
  DoorDefault = 4,   ///< Placeholder for future door tinting.

  DamageBase = 5,      ///< Base damage type color.
  DamageElectric = 6,  ///< Electric damage type color.
  DamageFire = 7,      ///< Fire damage type color.
  DamagePoison = 8,    ///< Poison damage type color.

  HealthGood = 9,       ///< Health bar fill above the warning threshold.
  HealthWarn = 10,      ///< Health bar fill at or below 50 health.
  HealthCritical = 11,  ///< Health bar fill at or below 20 health.
  BarEmpty = 12,        ///< Unfilled remainder of any bar.
  Shield = 13,          ///< Reserved for the shield overlay bar.

  EntityHit = 14,  ///< One-shot red flash on any entity (player or enemy) that
                   ///< just took damage.
};

/**
 * @brief Map a damage type to the color its projectiles/effects render in.
 *
 * @param type The damage type to look up.
 * @return The color pair registered for that damage type.
 */
ColorPair colorForDamageType(DamageType type);

#endif
