#include "entities/enemy_factory.h"

#include <random>

#include "core/coordinate.h"
#include "core/services.h"
#include "entities/enemy.h"
#include "world/room.h"
#include "world/tile.h"

namespace enemy_factory {

std::vector<std::unique_ptr<Enemy>> rollForRoom(const Room& room,
                                                GameServices& services) {
  std::vector<std::unique_ptr<Enemy>> enemies;

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
  if (floorTiles.empty()) return enemies;

  // Pick one tile from each half so enemies are spread across the room and
  // are always guaranteed to land on a valid Floor tile regardless of shape.
  std::size_t half = floorTiles.size() / 2;
  std::uniform_int_distribution<std::size_t> firstHalf(0, half - 1);
  std::uniform_int_distribution<std::size_t> secondHalf(half,
                                                        floorTiles.size() - 1);
  Coordinate spawn1 = floorTiles[firstHalf(services.rng)];
  Coordinate spawn2 = floorTiles[secondHalf(services.rng)];

  enemies.push_back(std::make_unique<Enemy>(spawn1.x, spawn1.y, 'G'));
  enemies.push_back(std::make_unique<Enemy>(spawn2.x, spawn2.y, 'O'));
  return enemies;
}

}  // namespace enemy_factory
