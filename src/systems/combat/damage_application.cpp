#include "systems/combat/damage_application.h"

#include "objects/entities/entity.h"

namespace combat
{
void applyDamage(Entity& target, Damage damage) { target.takeDamage(damage); }
}  // namespace combat
