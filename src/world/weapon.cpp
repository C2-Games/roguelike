#include "world/weapon.h"

const std::unordered_map<int, WeaponAttributes> Weapon::typeAttributes = {
    {static_cast<int>(WeaponType::Basic),
     {10, 2, 15, ColorPair::WeaponBasic, "Basic Bolt"}},
    {static_cast<int>(WeaponType::Rapid),
     {6, 3, 10, ColorPair::WeaponRapid, "Rapid Dart"}},
    {static_cast<int>(WeaponType::Heavy),
     {20, 1, 20, ColorPair::WeaponHeavy, "Heavy Slug"}},
    {static_cast<int>(WeaponType::Sniper),
     {30, 4, 30, ColorPair::WeaponSniper, "Sniper Round"}},
};

Weapon::Weapon(WeaponType type) : type(type) {}

int Weapon::getDamage() const {
  return typeAttributes.at(static_cast<int>(type)).damage;
}

int Weapon::getSpeed() const {
  return typeAttributes.at(static_cast<int>(type)).speed;
}

int Weapon::getRange() const {
  return typeAttributes.at(static_cast<int>(type)).range;
}

ColorPair Weapon::getColor() const {
  return typeAttributes.at(static_cast<int>(type)).color;
}

const char* Weapon::getName() const {
  return typeAttributes.at(static_cast<int>(type)).name;
}
