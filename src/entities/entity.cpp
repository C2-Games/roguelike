#include "entities/entity.h"

#include <utility>

Entity::Entity(Coordinate position, EntitySymbol symbol, int health, int speed,
               std::unique_ptr<FOV> fov)
    : position_(position),
      symbol_(std::move(symbol)),
      health_(health),
      speed_(speed),
      frameCounter_(0),
      fov_(std::move(fov)),
      state_(EntityActionState::Idle)
{}
