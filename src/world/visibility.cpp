#include "world/visibility.h"

#include "entities/fov.h"
#include "world/room.h"

namespace visibility {

void update(Room& room, Coordinate origin, const FOV& fov) {
  room.clearVisible();
  for (const Coordinate& pos : fov.absoluteFOV(origin)) {
    room.reveal(pos.x, pos.y);
  }
}

}  // namespace visibility
