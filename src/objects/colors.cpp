#include "objects/colors.h"

ColorPair colorForDamageType(DamageType type)
{
  switch (type)
  {
    case DamageType::Base:
      return ColorPair::DamageBase;
    case DamageType::Poison:
      return ColorPair::DamagePoison;
    case DamageType::Electric:
      return ColorPair::DamageElectric;
    case DamageType::Fire:
      return ColorPair::DamageFire;
  }
  return ColorPair::DamageBase;
}
