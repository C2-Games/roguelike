#ifndef RENDER_STATE_H
#define RENDER_STATE_H

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "objects/colors.h"
#include "objects/coordinate.h"
#include "objects/entities/entity_symbol.h"
#include "objects/room/room_dimensions.h"

enum class TileVisibility : std::uint8_t
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

struct MapLayerPacket
{
  std::array<std::array<TileView, ROOM_HEIGHT>, ROOM_WIDTH> tiles;
};

struct EntityLayerPacket
{
  EntityView player;
  std::vector<EntityView> enemies;  // already alive- and visibility-filtered
  std::vector<ProjectileView>
      projectiles;  // already active- and visibility-filtered
};

struct HUDLayerPacket
{
  int playerHealth, playerMaxHealth;
  int roomIndex, roomCount;
  WeaponView weapon;
};

struct DebugLayerPacket
{
  Coordinate playerPosition;
  double fps;
};

struct RenderState
{
  MapLayerPacket map;
  EntityLayerPacket entity;
  HUDLayerPacket hud;
  DebugLayerPacket debug;
};

#endif
