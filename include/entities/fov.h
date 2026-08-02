#ifndef FOV_H
#define FOV_H

#include <set>
#include <vector>

#include "core/coordinate.h"

class FOV {
 public:
  std::set<Coordinate> offsets;

  /**
   * @brief Construct a new FOV.
   *
   * @param offsets Set of offset positions (from origin) that defines the
   * FOV.
   */
  explicit FOV(std::set<Coordinate> offsets);

  /**
   * @brief Determine if a position is in an origin's FOV.
   *
   * @param origin The position of origin with this FOV.
   * @param position The position of the target.
   * @return bool True if position is in FOV (based on origin).
   */
  bool in(Coordinate origin, Coordinate position) const;

  /**
   * @brief Return all absolute positions covered by FOV from origin.
   *
   * @param origin The position of origin with this FOV.
   * @return std::vector<Coordinate>
   */
  std::vector<Coordinate> absoluteFOV(Coordinate origin) const;
};

/**
 * @brief Create a filled ellipse FOV, compensating for terminal aspect ratio.
 *
 * @param rx Radius in columns (horizontal).
 * @param ry Radius in rows (vertical). NOTE: row heights are roughly equiv. to
 * 2-3 column widths.
 * @return FOV
 */
FOV ellipseFOV(int rx, int ry);

#endif