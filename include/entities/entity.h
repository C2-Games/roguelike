#ifndef ENTITY_H
#define ENTITY_H

#include <cstdint>
#include <memory>

#include "core/coordinate.h"
#include "entities/entity_symbol.h"
#include "entities/fov.h"

enum class EntityActionState : std::uint8_t
{
  Attack,
  Move,
  Idle,
  Damaged,
  Ability,
  TransRoom,
};

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
   * @brief Get the entity's current action state.
   *
   * @return The entity's current action this frame.
   */
  EntityActionState getActionState() const { return actionState_; };

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
  EntitySymbol symbol_;
  int health_;
  int speed_;
  int frameCounter_;
  std::unique_ptr<FOV> fov_;
  EntityActionState actionState_;

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
      if (!(newPos == position_))
      {
        setActionState(EntityActionState::Move);
      }
      position_ = newPos;
    };
  };

  /**
   * @brief Set entities action state.
   *
   * @param state The action state to set.
   */
  void setActionState(EntityActionState state) { actionState_ = state; };
};

#endif