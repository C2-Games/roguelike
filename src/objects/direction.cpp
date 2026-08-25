#include "objects/direction.h"

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
