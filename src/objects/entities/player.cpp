#include "objects/entities/player.h"

#include "objects/coordinate.h"
#include "objects/entities/entity.h"
#include "objects/fovs/ellipse_fov.h"

Player::Player(Coordinate position, int health, int speed)
    : Entity(position, EntitySymbol{{'@'}}, health, speed,
             std::make_unique<EllipseFOV>(16, 10)),
      maxHealth_(health)
{}

void Player::changeFOV(int rx, int ry)
{
  fov_ = std::make_unique<EllipseFOV>(rx, ry);
}