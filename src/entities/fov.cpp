#include "entities/fov.h"

#include <algorithm>
#include <iterator>
#include <typeinfo>
#include <utility>

FOV::FOV(std::set<Coordinate> offsets) : offsets_(std::move(offsets))
{
  static const std::vector<Coordinate> CARDINALS = {
      Coordinate(0, -1), Coordinate(0, 1), Coordinate(-1, 0), Coordinate(1, 0)};
  for (const Coordinate& dir : CARDINALS)
  {
    std::vector<Coordinate>& bucket = leaving_[dir];
    std::copy_if(offsets_.begin(), offsets_.end(), std::back_inserter(bucket),
                 [this, &dir](const Coordinate& s) {
                   return offsets_.count(s - dir) == 0;
                 });
  }
}

FOV::~FOV() = default;

bool FOV::in(Coordinate origin, Coordinate position) const
{
  // must mirror absoluteFOV, which builds positions as origin + offset.
  Coordinate offset = position - origin;
  return offsets_.count(offset) > 0;
}

std::vector<Coordinate> FOV::absoluteFOV(Coordinate origin) const
{
  // initialize a vector of positions w/ size of number of offsets.
  std::vector<Coordinate> positions;
  positions.reserve(offsets_.size());

  // iterate through & get the absolute position based on origin pos.
  std::transform(
      offsets_.begin(), offsets_.end(), std::back_inserter(positions),
      [&origin](const Coordinate& offset) { return origin + offset; });

  return positions;
}

const std::vector<Coordinate>* FOV::leavingOffsets(Coordinate dir) const
{
  auto dirEntry = leaving_.find(dir);
  if (dirEntry == leaving_.end()) return nullptr;
  return &dirEntry->second;
}

bool FOV::equals(const FOV& other) const
{
  return typeid(*this) == typeid(other);
}
