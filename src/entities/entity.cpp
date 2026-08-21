#include "entities/entity.h"

#include <cstddef>
#include <utility>

Entity::Entity(Coordinate position, EntitySymbol symbol, int health, int speed,
               std::unique_ptr<FOV> fov)
    : position_(position),
      symbol_(std::move(symbol)),
      health_(health),
      speed_(speed),
      frameCounter_(0),
      fov_(std::move(fov))
{}

std::vector<Coordinate> Entity::occupiedTiles(Coordinate origin) const
{
  std::vector<Coordinate> tiles;
  for (std::size_t row = 0; row < symbol_.size(); ++row)
  {
    for (std::size_t col = 0; col < symbol_[row].size(); ++col)
    {
      if (symbol_[row][col] == '\0') continue;
      tiles.push_back(origin +
                      Coordinate(static_cast<int>(col), static_cast<int>(row)));
    }
  }
  return tiles;
}

bool Entity::occupies(Coordinate origin, Coordinate pos) const
{
  Coordinate offset = pos - origin;
  if (offset.y < 0 || offset.y >= static_cast<int>(symbol_.size()))
    return false;
  const std::vector<char>& row = symbol_[offset.y];
  if (offset.x < 0 || offset.x >= static_cast<int>(row.size())) return false;
  return row[offset.x] != '\0';
}
