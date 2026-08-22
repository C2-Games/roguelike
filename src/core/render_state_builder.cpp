#include "core/render_state_builder.h"

#include "entities/enemy.h"
#include "entities/player.h"
#include "world/map/level.h"
#include "world/map/room.h"
#include "world/objects/projectile.h"
#include "world/objects/weapon.h"

namespace render_state_builder
{

RenderState build(const Player& player, const Level& level,
                  const std::vector<std::unique_ptr<Projectile>>& projectiles,
                  double fps, bool playerHitFlashActive)
{
  RenderState state;

  const Room& room = level.getCurrentRoom();
  for (int x = 0; x < Room::WIDTH; ++x)
  {
    for (int y = 0; y < Room::HEIGHT; ++y)
    {
      TileView& tile = state.map.tiles[x][y];
      tile.symbol = room.tiles[x][y].getSymbol();

      if (room.isVisible(x, y))
      {
        tile.visibility = TileVisibility::Visible;
      }
      else if (room.isExplored(x, y))
      {
        tile.visibility = TileVisibility::Explored;
      }
      else
      {
        tile.visibility = TileVisibility::Unseen;
      }
    }
  }

  // an empty symbol draws nothing: render() can still run once on the frame
  // the player dies (Game::run()'s loop only rechecks isAlive() at the top
  // of the next iteration), so this keeps that frame's player invisible.
  state.entity.player =
      EntityView{player.getPosition(),
                 player.isAlive() ? player.getSymbol() : EntitySymbol{},
                 playerHitFlashActive, ColorPair::PlayerHit};

  for (const auto& enemy : room.enemies())
  {
    const Coordinate& position = enemy->getPosition();
    if (enemy->isAlive() && room.isVisible(position.x, position.y))
    {
      state.entity.enemies.push_back(
          EntityView{position, enemy->getSymbol(), false, ColorPair::Default});
    }
  }

  for (const auto& projectile : projectiles)
  {
    const Coordinate& position = projectile->getPosition();
    if (projectile->isActive() && room.isVisible(position.x, position.y))
    {
      state.entity.projectiles.push_back(
          ProjectileView{position, projectile->getColor()});
    }
  }

  state.hud.playerHealth = player.getHealth();
  state.hud.playerMaxHealth = player.getMaxHealth();
  state.hud.roomIndex = level.getCurrentRoomID();
  state.hud.roomCount = level.getRoomCount();

  const Weapon& weapon = player.getWeapon();
  state.hud.weapon =
      WeaponView{weapon.getName(), weapon.getDamage(), weapon.getSpeed(),
                 weapon.getRange(), weapon.getColor()};

  state.debug.playerPosition = player.getPosition();
  state.debug.fps = fps;

  return state;
}

}  // namespace render_state_builder
