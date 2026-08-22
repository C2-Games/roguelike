#include "entities/ellipse_fov.h"

#include <cmath>
#include <memory>
#include <set>
#include <utility>

EllipseFOV::EllipseFOV(int rx, int ry)
    : FOV(computeOffsets(rx, ry)), rx_(rx), ry_(ry)
{}

std::unique_ptr<FOV> EllipseFOV::clone() const
{
  return std::make_unique<EllipseFOV>(rx_, ry_);
}

bool EllipseFOV::equals(const FOV& other) const
{
  if (!FOV::equals(other))
  {
    return false;
  }
  const auto& otherEllipse = static_cast<const EllipseFOV&>(other);
  return rx_ == otherEllipse.rx_ && ry_ == otherEllipse.ry_;
}

std::set<Coordinate> EllipseFOV::computeOffsets(int rx, int ry)
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
    int dx_max = static_cast<int>(static_cast<float>(rx) *
                                  std::sqrt(1.0F - (y_norm * y_norm)));

    // iterate through -dx --> dx.
    for (int dx = -dx_max; dx <= dx_max; ++dx)
    {
      offsets.insert(Coordinate(dx, dy));
    };
  }

  // cooked.
  return offsets;
}
