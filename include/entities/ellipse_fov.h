#ifndef ELLIPSE_FOV_H
#define ELLIPSE_FOV_H

#include <memory>
#include <set>

#include "core/coordinate.h"
#include "entities/fov.h"

class EllipseFOV : public FOV
{
 public:
  /**
   * @brief Construct a new filled ellipse FOV, compensating for terminal
   * aspect ratio.
   *
   * @param rx Radius in columns (horizontal).
   * @param ry Radius in rows (vertical). NOTE: row heights are roughly
   * equiv. to 2-3 column widths.
   */
  explicit EllipseFOV(int rx, int ry);

  /**
   * @brief Create an independent copy of this ellipse FOV.
   *
   * @return std::unique_ptr<FOV> A new owned EllipseFOV with the same radii.
   */
  std::unique_ptr<FOV> clone() const override;

 protected:
  /**
   * @brief Equality by construction radii.
   *
   * @param other The FOV to compare against.
   * @return bool True if both FOVs are ellipses constructed with the same
   * radii.
   */
  bool equals(const FOV& other) const override;

 private:
  int rx_;
  int ry_;

  static std::set<Coordinate> computeOffsets(int rx, int ry);
};

#endif
