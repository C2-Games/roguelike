#ifndef FOV_H
#define FOV_H

#include <map>
#include <memory>
#include <set>
#include <vector>

#include "core/coordinate.h"

class FOV
{
 public:
  virtual ~FOV();

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
   * @brief Create an independent copy of this FOV.
   *
   * @return std::unique_ptr<FOV> A new owned FOV of the same concrete kind.
   */
  virtual std::unique_ptr<FOV> clone() const = 0;

  /**
   * @brief Equality by concrete shape.
   *
   * @param other The FOV to compare against.
   * @return bool True if both FOVs are the same concrete kind and shape.
   */
  bool operator==(const FOV& other) const { return equals(other); }

 protected:
  /**
   * @brief Construct a new FOV from a precomputed offset set, common to every
   * concrete shape.
   *
   * @param offsets Set of offset positions (from origin) that defines the
   * FOV.
   */
  explicit FOV(std::set<Coordinate> offsets);

  /**
   * @brief Base equality check: same concrete kind.
   *
   * @param other The FOV to compare against.
   * @return bool True if other is the same concrete kind as this FOV.
   */
  virtual bool equals(const FOV& other) const;

 private:
  std::set<Coordinate> offsets_;
  std::map<Coordinate, std::vector<Coordinate>> leaving_;
};

#endif
