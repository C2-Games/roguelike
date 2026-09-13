#ifndef WEAPON_H
#define WEAPON_H

#include "objects/colors.h"
#include "objects/damage/damage.h"

// flat weapon stat block.
struct Weapon
{
  Damage damage;
  ColorPair color;
  const char* name;
  int range;
  int fireRate;
  int tier;
  int speed;
  char symbol;
  char ammoSymbol;
};

#endif
