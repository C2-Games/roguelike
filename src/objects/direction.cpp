#include "objects/direction.h"

#include <cstdlib>

Coordinate toOffset(Direction direction)
{
  switch (direction)
  {
    case Direction::North:
      return Coordinate(0, -1);
    case Direction::South:
      return Coordinate(0, 1);
    case Direction::East:
      return Coordinate(1, 0);
    case Direction::West:
      return Coordinate(-1, 0);
  }
  return Coordinate(0, 0);
}

Direction directionTowards(Coordinate from, Coordinate to)
{
  int dx = to.x - from.x;
  int dy = to.y - from.y;

  if (std::abs(dx) >= std::abs(dy))
  {
    return dx >= 0 ? Direction::East : Direction::West;
  }
  return dy >= 0 ? Direction::South : Direction::North;
}
