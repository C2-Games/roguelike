#include "systems/combat/damage_application.h"

#include <algorithm>

#include "objects/entities/enemy.h"
#include "objects/entities/entity.h"
#include "objects/room/room.h"

namespace combat
{
void applyDamage(Entity& target, Damage damage)
{
  target.setHealth(std::max(target.getHealth() - damage.amount, 0));
  target.setActionState(EntityActionState::Damaged);
}

void reapDead(Room& room, std::vector<std::unique_ptr<Enemy>>& active)
{
  for (const auto& enemy : active)
  {
    if (!enemy->isAlive())
    {
      room.toggleOccupied(enemy->getPosition(), false);
    }
  }
  active.erase(std::remove_if(active.begin(), active.end(),
                              [](const std::unique_ptr<Enemy>& enemy) {
                                return !enemy->isAlive();
                              }),
               active.end());
}
}  // namespace combat
