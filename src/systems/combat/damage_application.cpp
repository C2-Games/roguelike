#include "systems/combat/damage_application.h"

#include <algorithm>

#include "objects/entities/enemy.h"
#include "objects/entities/entity.h"
#include "objects/room/room.h"

namespace
{
// duration, in frames, that an entity's hit-flash stays visible.
constexpr int HIT_FLASH_FRAMES = 8;
}  // namespace

namespace combat
{
void applyDamage(Entity& target, Damage damage)
{
  target.setHealth(std::max(target.getHealth() - damage.amount, 0));
  target.setActionState(EntityActionState::Damaged);
  target.triggerHitFlash(HIT_FLASH_FRAMES);
}

void reapDead(Room& room, std::vector<std::unique_ptr<Enemy>>& active)
{
  active.erase(std::remove_if(active.begin(), active.end(),
                              [&room](const std::unique_ptr<Enemy>& enemy) {
                                if (enemy->isAlive())
                                {
                                  return false;
                                }
                                room.toggleOccupied(enemy->getPosition(),
                                                    false);
                                return true;
                              }),
               active.end());
}
}  // namespace combat
