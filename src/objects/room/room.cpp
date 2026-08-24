#include "objects/room/room.h"

#include <utility>

Room::Room(int id) : roomID_(id), tiles_(WIDTH, std::vector<Tile>(HEIGHT)) {}

void Room::setTile(Coordinate pos, Tile tile)
{
  if (!inBounds(pos))
  {
    return;
  }
  tiles_[pos.x][pos.y] = std::move(tile);
}

bool Room::isVisible(Coordinate pos) const
{
  if (!inBounds(pos))
  {
    return false;
  }
  return tiles_[pos.x][pos.y].isRevealed();
}

bool Room::isExplored(Coordinate pos) const
{
  if (!inBounds(pos))
  {
    return false;
  }
  return tiles_[pos.x][pos.y].isExplored();
}

void Room::toggleReveal(Coordinate pos, bool visible)
{
  if (!inBounds(pos))
  {
    return;
  }
  tiles_[pos.x][pos.y].toggleReveal(visible);
}

bool Room::isOccupied(Coordinate pos) const
{
  if (!inBounds(pos))
  {
    return false;
  }
  return tiles_[pos.x][pos.y].isOccupied();
}

void Room::toggleOccupied(Coordinate pos, bool occupied)
{
  if (!inBounds(pos))
  {
    return;
  }
  tiles_[pos.x][pos.y].toggleOccupied(occupied);
}

bool Room::isWalkable(Coordinate pos) const
{
  if (!inBounds(pos))
  {
    return false;
  }
  return tiles_[pos.x][pos.y].isWalkable();
}

TileType Room::getTileType(Coordinate pos) const
{
  if (!inBounds(pos))
  {
    return TileType::Wall;
  }
  return tiles_[pos.x][pos.y].getType();
}

char Room::getTileSymbol(Coordinate pos) const
{
  if (!inBounds(pos))
  {
    return ' ';
  }
  return tiles_[pos.x][pos.y].getSymbol();
}

void Room::clearVisible()
{
  for (auto& col : tiles_)
  {
    for (auto& tile : col)
    {
      tile.toggleReveal(false);
    }
  }
}
