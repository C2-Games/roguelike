#include "world/level.h"

#include "world/room.h"

Level::Level(int roomCount, GameServices& services)
    : roomGraph_(roomCount, services), enemyRegistry_(services) {}

void Level::loadInitialEnemies(
    std::vector<std::unique_ptr<Enemy>>& activeEnemies) {
  enemyRegistry_.loadForRoom(0, roomGraph_.getRoom(0), activeEnemies);
}

void Level::transitionEnemies(
    int fromRoomID, int toRoomID,
    std::vector<std::unique_ptr<Enemy>>& activeEnemies) {
  enemyRegistry_.transitionActive(fromRoomID, toRoomID,
                                  roomGraph_.getRoom(toRoomID), activeEnemies);
}

void Level::updateVisibility(Coordinate origin, const FOV& fov) {
  Room& room = roomGraph_.getCurrentRoom();

  // Wipe last frame's visibility, then light up the current FoV cells.
  // reveal() is bounds-checked so out-of-room FoV offsets are safely
  // ignored.
  room.clearVisible();
  for (const Coordinate& pos : fov.absoluteFOV(origin)) {
    room.reveal(pos.x, pos.y);
  }
}

const GoalMap& Level::getGoalMap(int roomID, Coordinate goal) const {
  return goalMapCache_.getOrCompute(roomGraph_.getRoom(roomID), goal);
}
