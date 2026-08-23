#include "world/map/room.h"

#include <stdexcept>
#include <string>

#include "entities/enemy.h"
#include "entities/fov.h"
#include "entities/player.h"

Room::Room(int id) : roomID(id), tiles(WIDTH, std::vector<Tile>(HEIGHT)) {}

void Room::updateVisibility(Coordinate origin, const FOV& fov)
{
  clearVisible();
  for (const Coordinate& pos : fov.absoluteFOV(origin))
  {
    reveal(pos.x, pos.y);
  }
}

bool Room::updateVisibilityDelta(Coordinate previousOrigin, Coordinate origin,
                                 const FOV& fov)
{
  Coordinate dir = origin - previousOrigin;
  const std::vector<Coordinate>* leaving = fov.leavingOffsets(dir);
  const std::vector<Coordinate>* entering =
      fov.leavingOffsets(Coordinate(-dir.x, -dir.y));
  if (leaving == nullptr || entering == nullptr)
  {
    return false;
  }

  for (const Coordinate& offset : *leaving)
  {
    Coordinate pos = previousOrigin + offset;
    if (pos.x < 0 || pos.x >= WIDTH || pos.y < 0 || pos.y >= HEIGHT)
    {
      continue;
    }
    // No bounds-checked equivalent of reveal() exists for clearing a single
    // tile, so this open-codes the same bounds check reveal() does.
    tiles[pos.x][pos.y].clearVisible();
  }

  for (const Coordinate& offset : *entering)
  {
    Coordinate pos = origin + offset;
    reveal(pos.x, pos.y);
  }

  return true;
}

void Room::updateVisibility(Coordinate previousOrigin, Coordinate origin,
                            const FOV& fov)
{
  if (!updateVisibilityDelta(previousOrigin, origin, fov))
  {
    updateVisibility(origin, fov);
  }
}

void Room::clearVisible()
{
  for (auto& col : tiles)
  {
    for (auto& tile : col)
    {
      tile.clearVisible();
    }
  }
}

bool Room::isVisible(int x, int y) const
{
  if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
  {
    return false;
  }
  return tiles[x][y].isVisible();
}

bool Room::isExplored(int x, int y) const
{
  if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
  {
    return false;
  }
  return tiles[x][y].isExplored();
}

void Room::reveal(int x, int y)
{
  if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
  {
    return;
  }
  tiles[x][y].reveal();
}

Coordinate Room::doorAt(DoorNumber number) const
{
  auto doorEntry = doors.find(number);
  if (doorEntry == doors.end())
  {
    throw std::runtime_error("room " + std::to_string(roomID) + " (" + name +
                             ") has no door " + std::to_string(number));
  }
  return doorEntry->second;
}

Coordinate Room::inwardOfDoor(Coordinate doorPos)
{
  Coordinate inward = doorPos;
  if (doorPos.x == 0)
  {
    inward.x = 1;
  }
  else if (doorPos.x == Room::WIDTH - 1)
  {
    inward.x = Room::WIDTH - 2;
  }
  else if (doorPos.y == 0)
  {
    inward.y = 1;
  }
  else if (doorPos.y == Room::HEIGHT - 1)
  {
    inward.y = Room::HEIGHT - 2;
  }
  return inward;
}

const Entity* Room::entityAt(Coordinate pos, const Player& player) const
{
  if (Enemy* enemy = enemyAt(pos))
  {
    return enemy;
  }
  if (player.isAlive() && player.getPosition() == pos)
  {
    return &player;
  }
  return nullptr;
}

Entity* Room::entityAt(Coordinate pos, Player& player) const
{
  return const_cast<Entity*>(entityAt(pos, const_cast<const Player&>(player)));
}
