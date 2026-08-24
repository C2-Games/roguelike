#ifndef DAMAGE_H
#define DAMAGE_H

#include "objects/damage/damage_type.h"

struct Damage
{
  DamageType type;
  int amount;
  double duration;
};

#endif
