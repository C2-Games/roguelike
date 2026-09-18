#include "objects/weapons/projectile.h"

#include <algorithm>
#include <iterator>

#include "objects/colors.h"

Projectile::Projectile(Coordinate position, Direction direction, Damage damage,
                       int tilesPerTick, int range, char ammoSymbol,
                       const double (&crit)[5])
    : position_(position),
      direction_(direction),
      damage_(damage),
      tilesPerTick_(tilesPerTick),
      remainingRange_(range),
      color_(colorForDamageType(damage.type)),
      ammoSymbol_(ammoSymbol)
{
  std::copy(std::begin(crit), std::end(crit), crit_);
}
