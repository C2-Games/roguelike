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

const GoalMap& Level::getGoalMap(int roomID, Coordinate goal) const {
  return goalMapCache_.getOrCompute(roomGraph_.getRoom(roomID), goal);
}
