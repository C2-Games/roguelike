
#include "entities/fov.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <set>
#include <utility>
#include <vector>

FOV::FOV(std::set<Coordinate> offsets, int rx, int ry)
    : offsets(std::move(offsets)), rx_(rx), ry_(ry)
{
  static const std::vector<Coordinate> CARDINALS = {
      Coordinate(0, -1), Coordinate(0, 1), Coordinate(-1, 0), Coordinate(1, 0)};
  for (const Coordinate& dir : CARDINALS)
  {
    std::vector<Coordinate>& bucket = leaving_[dir];
    std::copy_if(this->offsets.begin(), this->offsets.end(),
                 std::back_inserter(bucket), [this, &dir](const Coordinate& s) {
                   return this->offsets.count(s - dir) == 0;
                 });
  }
}

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

const std::vector<Coordinate>* FOV::leavingOffsets(Coordinate dir) const
{
  auto dirEntry = leaving_.find(dir);
  if (dirEntry == leaving_.end()) return nullptr;
  return &dirEntry->second;
}

bool FOV::operator==(const FOV& other) const
{
  return rx_ == other.rx_ && ry_ == other.ry_;
}

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
  return FOV(offsets, rx, ry);
}