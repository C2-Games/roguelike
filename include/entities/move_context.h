#ifndef MOVE_CONTEXT_H
#define MOVE_CONTEXT_H

#include <memory>
#include <vector>

#include "core/coordinate.h"

class Enemy;
class GoalMapCache;
struct GameServices;
struct Room;

struct MoveContext {
  Coordinate playerPos;
  const Room& room;
  GoalMapCache& cache;
  const std::vector<std::unique_ptr<Enemy>>& allEnemies;
  GameServices& services;
};

#endif
