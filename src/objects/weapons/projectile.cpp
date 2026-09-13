#include "objects/weapons/projectile.h"

#include "objects/colors.h"

Projectile::Projectile(Coordinate position, Direction direction, Damage damage,
                       int tilesPerTick, int range, char ammoSymbol)
    : position_(position),
      direction_(direction),
      damage_(damage),
      tilesPerTick_(tilesPerTick),
      remainingRange_(range),
      color_(colorForDamageType(damage.type)),
      ammoSymbol_(ammoSymbol)
{}
