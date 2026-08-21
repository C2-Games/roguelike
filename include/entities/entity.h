#ifndef ENTITY_H
#define ENTITY_H

#include <memory>
#include <vector>

#include "core/coordinate.h"
#include "entities/entity_symbol.h"
#include "entities/fov.h"

class Entity
{
 public:
  Entity(Coordinate position, EntitySymbol symbol, int health, int speed,
         std::unique_ptr<FOV> fov);

  virtual ~Entity() = default;

  /**
   * @brief Get the entity health.
   *
   * @return const int
   */
  int getHealth() const { return health_; };

  /**
   * @brief Get the entities symbol.
   *
   * @return const EntitySymbol&
   */
  const EntitySymbol& getSymbol() const { return symbol_; };

  /**
   * @brief Get the entity position.
   *
   * @return const Coordinate
   */
  Coordinate getPosition() const { return position_; };

  /**
   * @brief Get entity speed.
   *
   * @return const int [frames / move]
   */
  int getSpeed() const { return speed_; };

  /**
   * @brief Get the entity's field-of-view mask.
   *
   * @return const FOV&
   */
  const FOV& getFOV() const { return *fov_; }

  /**
   * @brief Entity is alive.
   *
   * @return bool
   */
  bool isAlive() const { return health_ > 0; };

  /**
   * @brief Move entity to new position.
   *
   * @param newPos Corrdinates of new position.
   */
  void moveTo(Coordinate newPos) { moveHook(newPos); };

  /**
   * @brief Every absolute tile this entity's symbol covers, offset from a
   * given origin.
   *
   * @param origin World position to offset the symbol from.
   * @return std::vector<Coordinate> One entry per non-transparent ('\0')
   * cell.
   */
  std::vector<Coordinate> occupiedTiles(Coordinate origin) const;

  /**
   * @brief This entity's currently-occupied tiles, at its current position.
   *
   * @return std::vector<Coordinate> One entry per non-transparent ('\0')
   * cell.
   */
  std::vector<Coordinate> occupiedTiles() const
  {
    return occupiedTiles(position_);
  }

  /**
   * @brief Whether this entity's symbol, offset from origin, covers pos.
   *
   * @param origin World position to offset the symbol from.
   * @param pos Tile to test.
   * @return bool True if a non-transparent ('\0') symbol cell lands on pos.
   */
  bool occupies(Coordinate origin, Coordinate pos) const;

  // abstract methods.
  virtual void takeDamage(int damage) = 0;

 protected:
  Coordinate position_;
  EntitySymbol symbol_;
  int health_;
  int speed_;
  int frameCounter_;
  std::unique_ptr<FOV> fov_;

  /**
   * @brief Move hook that moves player to new position based on their speed.
   *
   * @param newPos The potential new position to move entitiy.
   */
  void moveHook(Coordinate newPos)
  {
    // when move hook is called, we assume a frame.
    frameCounter_ += 1;

    if (frameCounter_ % speed_ == 0)
    {
      frameCounter_ = 0;  // reset counter.
      position_ = newPos;
    };
  };
};

#endif