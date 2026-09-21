#include "objects/entities/entity.h"

#include <utility>

Entity::Entity(Coordinate position, EntitySymbol symbol, int health, int speed,
               std::unique_ptr<FOV> fov, const Weapon& weapon)
    : position_(position),
      symbol_(std::move(symbol)),
      health_(health),
      speed_(speed),
      frameCounter_(0),
      fov_(std::move(fov)),
      actionState_(EntityActionState::Idle),
      hitFlashFramesRemaining_(0),
      weapon_(weapon),
      crit_{}
{}
