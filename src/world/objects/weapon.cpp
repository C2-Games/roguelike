#include "world/objects/weapon.h"

const std::unordered_map<int, WeaponAttributes> Weapon::typeAttributes_ = {
    {static_cast<int>(WeaponType::Basic),
     {10, 2, 15, ColorPair::WeaponBasic, "Basic Bolt"}},
    {static_cast<int>(WeaponType::Rapid),
     {6, 3, 10, ColorPair::WeaponRapid, "Rapid Dart"}},
    {static_cast<int>(WeaponType::Heavy),
     {20, 1, 20, ColorPair::WeaponHeavy, "Heavy Slug"}},
    {static_cast<int>(WeaponType::Sniper),
     {30, 4, 30, ColorPair::WeaponSniper, "Sniper Round"}},
};

Weapon::Weapon(WeaponType type) : type_(type) {}

int Weapon::getDamage() const {
  return typeAttributes_.at(static_cast<int>(type_)).damage;
}

int Weapon::getSpeed() const {
  return typeAttributes_.at(static_cast<int>(type_)).speed;
}

int Weapon::getRange() const {
  return typeAttributes_.at(static_cast<int>(type_)).range;
}

ColorPair Weapon::getColor() const {
  return typeAttributes_.at(static_cast<int>(type_)).color;
}

const char* Weapon::getName() const {
  return typeAttributes_.at(static_cast<int>(type_)).name;
}
