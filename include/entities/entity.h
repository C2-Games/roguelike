#ifndef ENTITY_H
#define ENTITY_H

#include "core/coordinate.h"

class Entity {
 public:
  Entity(int x, int y, char symbol, int health, int speed);

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
   * @return const char
   */
  char getSymbol() const { return symbol_; };

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

  // abstract methods.
  virtual void takeDamage(int damage) = 0;

 protected:
  Coordinate position_;
  char symbol_;
  int health_;
  int speed_;
  int frameCounter_;

  /**
   * @brief Move hook that moves player to new position based on their speed.
   *
   * @param newPos The potential new position to move entitiy.
   */
  void moveHook(Coordinate newPos) {
    // when move hook is called, we assume a frame.
    frameCounter_ += 1;

    if (frameCounter_ % speed_ == 0) {
      frameCounter_ = 0;  // reset counter.
      position_ = newPos;
    };
  };
};

#endif