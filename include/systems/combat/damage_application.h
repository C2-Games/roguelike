#ifndef DAMAGE_APPLICATION_H
#define DAMAGE_APPLICATION_H

#include <memory>
#include <vector>

#include "objects/damage/damage.h"

class Entity;
class Enemy;

namespace combat
{
// Apply Damage to a defending entity.
void applyDamage(Entity& target, Damage damage);

// drop dead enemies from `active`.
void reapDead(std::vector<std::unique_ptr<Enemy>>& active);
}  // namespace combat

#endif
