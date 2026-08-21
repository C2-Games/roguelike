#ifndef RENDER_STATE_H
#define RENDER_STATE_H

#include <array>
#include <string>
#include <vector>

#include "core/colors.h"
#include "core/coordinate.h"
#include "entities/entity_symbol.h"
#include "world/map/room.h"

enum class TileVisibility
{
  Visible,
  Explored,
  Unseen
};

struct TileView
{
  char symbol;
  TileVisibility visibility;
};

struct EntityView
{
  Coordinate position;
  EntitySymbol symbol;
  bool tinted;          // whether to apply tintColor over the symbol this frame
  ColorPair tintColor;  // meaningful only when tinted == true
};

struct ProjectileView
{
  Coordinate position;
  ColorPair color;
};

struct WeaponView
{
  std::string name;
  int damage, speed, range;
  ColorPair color;
};

struct RenderState
{
  std::array<std::array<TileView, Room::HEIGHT>, Room::WIDTH> tiles;
  EntityView player;
  std::vector<EntityView> enemies;          // already alive-filtered
  std::vector<ProjectileView> projectiles;  // already active-filtered
  int playerHealth, playerMaxHealth;
  int roomIndex, roomCount;
  WeaponView weapon;
  double fps;
};

#endif
