#include "objects/weapons/projectile.h"

Projectile::Projectile(Coordinate position, Direction direction, Damage damage,
                       int tilesPerTick, int range, ColorPair color)
    : position_(position),
      direction_(direction),
      damage_(damage),
      tilesPerTick_(tilesPerTick),
      remainingRange_(range),
      color_(color)
{}
