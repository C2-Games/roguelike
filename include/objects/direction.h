#ifndef DIRECTION_H
#define DIRECTION_H

#include <array>

#include "objects/coordinate.h"

enum class Direction
{
  North,
  South,
  East,
  West
};

/**
 * @brief Convert a direction into the coordinate offset it represents.
 *
 * @param direction The direction to convert.
 * @return The unit offset corresponding to the given direction.
 */
Coordinate toOffset(Direction direction);

inline constexpr std::array<Direction, 4> ALL_DIRECTIONS = {
    Direction::North, Direction::East, Direction::South, Direction::West};

#endif
