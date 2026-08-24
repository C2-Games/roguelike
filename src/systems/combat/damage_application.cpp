#include "systems/combat/damage_application.h"

#include <algorithm>

#include "objects/entities/enemy.h"
#include "objects/entities/entity.h"

namespace combat
{
void applyDamage(Entity& target, Damage damage) { target.takeDamage(damage); }

void reapDead(std::vector<std::unique_ptr<Enemy>>& active)
{
  active.erase(std::remove_if(active.begin(), active.end(),
                              [](const std::unique_ptr<Enemy>& enemy) {
                                return !enemy->isAlive();
                              }),
               active.end());
}
}  // namespace combat
