#ifndef ROOM_ENEMY_STATE_H
#define ROOM_ENEMY_STATE_H

#include <memory>
#include <vector>

#include "objects/entities/enemy.h"

// a room's enemy state: whether it has been rolled yet, and the live enemies
// currently in it. Plain data only -- spawn/query/reap logic lives in
// core/room_enemy_logic.h until the systems/ layer exists.
struct RoomEnemyState
{
  bool spawned = false;
  std::vector<std::unique_ptr<Enemy>> enemies;
};

#endif
