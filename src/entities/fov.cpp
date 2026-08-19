
#include "entities/fov.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <set>
#include <utility>
#include <vector>

FOV::FOV(std::set<Coordinate> offsets) : offsets(std::move(offsets)) {}

bool FOV::in(Coordinate origin, Coordinate position) const
{
  // must mirror absoluteFOV, which builds positions as origin + offset.
  Coordinate offset = position - origin;
  return offsets.count(offset) > 0;
};

std::vector<Coordinate> FOV::absoluteFOV(Coordinate origin) const
{
  // initialize a vector of positions w/ size of number of offsets.
  std::vector<Coordinate> positions;
  positions.reserve(offsets.size());

  // iterate through & get the absolute position based on origin pos.
  std::transform(
      offsets.begin(), offsets.end(), std::back_inserter(positions),
      [&origin](const Coordinate& offset) { return origin + offset; });

  return positions;
};

FOV ellipseFOV(int rx, int ry)
{
  // need to get all offsets to create an FOV.
  std::set<Coordinate> offsets;

  // iterate through -y --> y.
  for (int dy = -ry; dy <= ry; ++dy)
  {
    // find max x via ellipse eq. (w/ origin 0):
    //    1 = (dx / rx)^2 + (dy / ry)^2
    //    dx = rx * sqrt(1 - (dy / ry)^2)
    // could've used std::ceil here, but rounding down is fine w/ me.
    float y_norm = static_cast<float>(dy) / static_cast<float>(ry);
    int dx_max = static_cast<int>(rx * std::sqrt(1.0f - y_norm * y_norm));

    // iterate through -dx --> dx.
    for (int dx = -dx_max; dx <= dx_max; ++dx)
    {
      offsets.insert(Coordinate(dx, dy));
    };
  }

  // cooked.
  return FOV(offsets);
}