#ifndef FOV_H
#define FOV_H

#include <map>
#include <set>
#include <vector>

#include "core/coordinate.h"

class FOV
{
 public:
  std::set<Coordinate> offsets;

  /**
   * @brief Construct a new FOV.
   *
   * @param offsets Set of offset positions (from origin) that defines the
   * FOV.
   * @param rx Radius in columns used to construct offsets.
   * @param ry Radius in rows used to construct offsets.
   */
  explicit FOV(std::set<Coordinate> offsets, int rx, int ry);

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

  /**
   * @brief Look up the precomputed offsets that stop being visible after a
   * single unit-cardinal step in a given direction.
   *
   * @param dir One of the 4 unit cardinal directions: (0,-1), (0,1), (-1,0),
   * (1,0).
   * @return const std::vector<Coordinate>* Pointer to the leaving offsets, or
   * nullptr if dir isn't one of the 4 precomputed directions.
   */
  const std::vector<Coordinate>* leavingOffsets(Coordinate dir) const;

  /**
   * @brief Equality by construction radii.
   *
   * @param other The FOV to compare against.
   * @return bool True if both FOVs were constructed with the same radii.
   */
  bool operator==(const FOV& other) const;

 private:
  std::map<Coordinate, std::vector<Coordinate>> leaving_;
  int rx_;
  int ry_;
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