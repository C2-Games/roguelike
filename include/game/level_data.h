#ifndef LEVEL_DATA_H
#define LEVEL_DATA_H

#include <map>
#include <memory>
#include <vector>

#include "objects/entities/enemy.h"
#include "objects/map.h"
#include "objects/room/room.h"
#include "objects/weapons/projectile.h"
#include "preload/level_meta.h"

// one room's live, per-playthrough entities -- as opposed to Room's static,
// authored geometry.
struct RoomObjects
{
  std::vector<std::unique_ptr<Enemy>> enemies;
  std::vector<std::unique_ptr<Projectile>> projectiles;
};

// per-room live entities, keyed by room ID.
using RoomData = std::map<int, RoomObjects>;

// everything Game needs to run a level: static room geometry, the door
// graph linking rooms, and each room's live entities.
struct LevelData
{
  LevelMeta meta;
  RoomConnections roomConnections;
  RoomData roomData;
  std::map<int, Room> rooms;
};

#endif
