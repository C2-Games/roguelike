#include "world/level.h"

#include <cstdlib>
#include <memory>
#include <random>

#include "core/services.h"
#include "world/tile.h"

Level::Level(int roomCount, GameServices& services)
    : services(services), roomGraph_(roomCount, services) {}

void Level::spawnEnemiesForRoom(int roomID) {
  const Room& room = roomGraph_.getRoom(roomID);

  // Collect every Floor tile in the room. Scanning x-major means the first
  // half of the list naturally covers the left/top portion of the shape and
  // the second half covers the right/bottom — giving spatial spread.
  std::vector<Coordinate> floorTiles;
  for (int x = 0; x < Room::WIDTH; x++) {
    for (int y = 0; y < Room::HEIGHT; y++) {
      if (room.tiles[x][y].getType() == TileType::Floor) {
        floorTiles.push_back(Coordinate(x, y));
      }
    }
  }

  if (floorTiles.empty()) return;

  // Pick one tile from each half so enemies are spread across the room and
  // are always guaranteed to land on a valid Floor tile regardless of shape.
  std::size_t half = floorTiles.size() / 2;
  std::uniform_int_distribution<std::size_t> firstHalf(0, half - 1);
  std::uniform_int_distribution<std::size_t> secondHalf(half,
                                                        floorTiles.size() - 1);
  Coordinate spawn1 = floorTiles[firstHalf(services.rng)];
  Coordinate spawn2 = floorTiles[secondHalf(services.rng)];

  roomEnemies[roomID].push_back(
      std::make_unique<Enemy>(spawn1.x, spawn1.y, 'G'));
  roomEnemies[roomID].push_back(
      std::make_unique<Enemy>(spawn2.x, spawn2.y, 'O'));
}

void Level::loadInitialEnemies(
    std::vector<std::unique_ptr<Enemy>>& activeEnemies) {
  if (!roomVisited[0]) {
    roomVisited[0] = true;
    spawnEnemiesForRoom(0);
  }
  activeEnemies = std::move(roomEnemies[0]);
}

void Level::transitionEnemies(
    int fromRoomID, int toRoomID,
    std::vector<std::unique_ptr<Enemy>>& activeEnemies) {
  // Park the current room's enemies back into storage.
  roomEnemies[fromRoomID] = std::move(activeEnemies);

  // Generate enemies for the destination room on first visit.
  if (!roomVisited[toRoomID]) {
    roomVisited[toRoomID] = true;
    spawnEnemiesForRoom(toRoomID);
  }

  // Hand the destination room's enemies to the caller.
  activeEnemies = std::move(roomEnemies[toRoomID]);
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
