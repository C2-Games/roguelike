#include "entities/entity.h"

Entity::Entity(int x, int y, char symbol, int health, int speed)
    : position_(x, y),
      symbol_(symbol),
      health_(health),
      speed_(speed),
      frameCounter_(0)
{}
