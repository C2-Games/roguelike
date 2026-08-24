#ifndef DAMAGE_TYPE_H
#define DAMAGE_TYPE_H

#include <cstdint>

enum class DamageType : std::uint8_t
{
  Base,
  Poison,
  Electric,
  Fire,
};

#endif
