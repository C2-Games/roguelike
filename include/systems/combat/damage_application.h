#ifndef DAMAGE_APPLICATION_H
#define DAMAGE_APPLICATION_H

#include "objects/damage/damage.h"

class Entity;

namespace combat
{
// Apply Damage to a defending entity.
void applyDamage(Entity& target, Damage damage);
}  // namespace combat

#endif
