#ifndef DAMAGE_APPLICATION_H
#define DAMAGE_APPLICATION_H

#include <memory>
#include <vector>

#include "objects/damage/damage.h"

class Entity;
class Enemy;
struct Room;

namespace combat
{
// apply damage to a defending entity.
void applyDamage(Entity& target, Damage damage);

// kill `entity` outright if it is standing on a Void tile; no-op otherwise.
// idempotent, so it is safe to call every frame regardless of movement.
void applyTerrainDamage(Entity& entity, const Room& room);

// drop dead enemies from `active`, clearing their room occupancy first.
void reapDead(Room& room, std::vector<std::unique_ptr<Enemy>>& active);
}  // namespace combat

#endif
