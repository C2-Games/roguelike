#ifndef WEAPON_TYPE_H
#define WEAPON_TYPE_H

#include <cstdint>

enum class WeaponType : std::uint8_t
{
  Basic,
  Rapid,
  Heavy,
  Sniper
};

#endif
