#ifndef WEAPON_ATTRIBUTES_H
#define WEAPON_ATTRIBUTES_H

#include "objects/colors.h"

struct WeaponAttributes
{
  int damage;
  int speed;
  int range;
  ColorPair color;
  const char* name;
};

#endif
