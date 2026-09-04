#include "game/game.h"

#include <algorithm>
#include <chrono>
#include <optional>
#include <thread>

#include "io/input/game_commands.h"
#include "io/ui_manager.h"
#include "objects/direction.h"
#include "objects/entities/enemy.h"
#include "objects/weapons/projectile.h"
#include "preload/level_loader.h"
#include "preload/room_loader.h"
#include "systems/combat/combat.h"
#include "systems/visibility/visibility.h"

namespace
{
// fixed until a real seed-input/new-game flow exists: every launch should
// reproduce the same room/enemy layout for a given level config.
constexpr std::mt19937::result_type DEFAULT_SEED = 80085;
// how far short of the frame deadline the coarse sleep stops.
constexpr auto SLEEP_MARGIN = std::chrono::microseconds(1000);
}  // namespace

Game::Game(UIManager& uiManager, int fps)
    : fps_(fps),
      services_(DEFAULT_SEED),
      player_(Coordinate(Room::WIDTH / 2, Room::HEIGHT / 2)),
      levelData_(
          preload::loadLevel("assets/levels/level_1", "assets", services_)),
      currentRoomID_(levelData_.meta.startRoomID),
      uiManager_(uiManager)
{
  currentRoom().toggleOccupied(player_.getPosition(), true);
}

void Game::run()
{
  UIManager::showStartScreen();

  const auto frame_duration =
      std::chrono::duration_cast<std::chrono::steady_clock::duration>(
          getDuration());
  auto next_frame_time = std::chrono::steady_clock::now();

  while (state_ != GameState::End)
  {
    while (state_ == GameState::Play)
    {
      // -------- Frame start --------
      auto start = std::chrono::steady_clock::now();
      handleInput();
      update();
      render();

      // -------- Frame end --------
      next_frame_time += frame_duration;  // deadline accumulator

      auto now = std::chrono::steady_clock::now();
      if (now > next_frame_time + frame_duration)
      {
        // fell behind by more than a full frame  — resync to now with no
        // added delay.
        next_frame_time = now;
      }
      else if (now < next_frame_time - SLEEP_MARGIN)
      {
        std::this_thread::sleep_for(next_frame_time - now - SLEEP_MARGIN);
      }

      // spin-wait the remaining slop for exact deadline alignment.
      while (std::chrono::steady_clock::now() < next_frame_time)
      {
      }

      // for debug window.
      std::chrono::duration<double, std::milli> totalFrame =
          std::chrono::steady_clock::now() - start;

      currentFps_ =
          totalFrame.count() > 0.0 ? 1000.0 / totalFrame.count() : 0.0;

      if (!player_.isAlive())
      {
        setState(GameState::End);
      }
    }

    switch (state_)
    {
      // no GameCommand ever sets state_ to Start, and none reaches Pause
      // yet; both fall back to Play defensively rather than spinning the
      // outer loop with no input poll or render.
      case GameState::Start:
      case GameState::Pause:
        setState(GameState::Play);
        break;
      case GameState::TransLevel:
        // Game hardcodes a single level with no level-ordering/manifest
        // concept, so there is nowhere to transition to.
        setState(GameState::End);
        break;
      case GameState::Play:
      case GameState::End:
        // unreachable here: Play is handled by the inner while; End breaks
        // the outer while.
        break;
    }
  }

  UIManager::showGameOver();
}

void Game::handleInput()
{
  GameCommand command = uiManager_.pollInput();
  std::optional<Direction> direction;

  switch (command)
  {
    case GameCommand::Resize:
      // unreachable: UIManager::pollInput() already resolves resizes to
      // None before Game sees them. kept only so the switch stays
      // exhaustive over GameCommand.
      break;
    case GameCommand::MoveUp:
      direction = Direction::North;
      break;
    case GameCommand::MoveDown:
      direction = Direction::South;
      break;
    case GameCommand::MoveLeft:
      direction = Direction::West;
      break;
    case GameCommand::MoveRight:
      direction = Direction::East;
      break;
    case GameCommand::Attack:
      currentRoomObjects().projectiles.push_back(
          combat::spawnProjectile(player_));
      player_.setActionState(EntityActionState::Attack);
      return;
    case GameCommand::Quit:
      setState(GameState::End);
      break;
    case GameCommand::None:
      break;
  }

  if (!direction.has_value())
  {
    // safe to skip movement/door resolution entirely here: the player's own
    // tile is always in-bounds, walkable, and unoccupied, and re-resolving a
    // door connection at a fixed (room, position) pair is idempotent.
    return;
  }

  player_.setLastDirection(*direction);

  movement::PlayerStepOutcome outcome =
      movement::stepPlayer(player_, currentRoom(), *direction);
  if (outcome.kind == movement::PlayerStepKind::AtDoor)
  {
    auto connEntry = levelData_.roomConnections.find(
        DoorConnection{currentRoomID_, outcome.doorPos});
    if (connEntry != levelData_.roomConnections.end())
    {
      const DoorConnection& conn = connEntry->second;
      currentRoom().toggleOccupied(player_.getPosition(), false);
      currentRoomID_ = conn.roomID;
      Coordinate landing =
          room_loader::inwardOfDoor(currentRoom(), conn.doorPosition);
      player_.moveTo(landing);
      currentRoom().toggleOccupied(landing, true);
      player_.setActionState(EntityActionState::TransRoom);
    }
    else
    {
      currentRoom().toggleOccupied(player_.getPosition(), false);
      player_.moveTo(outcome.doorPos);
      currentRoom().toggleOccupied(outcome.doorPos, true);
    }
  }
}

void Game::update()
{
  // recompute FoV visibility for the current room before anything else runs
  // this frame, but only when it can have changed.
  bool sameRoomAndShape = currentRoomID_ == lastVisibilityRoomID_ &&
                          player_.getFOV() == *lastVisibilityFov_;
  if (playerMoved() || !sameRoomAndShape)
  {
    Room& room = currentRoom();
    if (sameRoomAndShape)
    {
      visibility::update(room, lastVisibilityPos_, player_.getPosition(),
                         player_.getFOV());
    }
    else
    {
      visibility::update(room, player_.getPosition(), player_.getFOV());
    }
    lastVisibilityPos_ = player_.getPosition();
    lastVisibilityRoomID_ = currentRoomID_;
    lastVisibilityFov_ = player_.getFOV().clone();
  }

  Room& room = currentRoom();
  RoomObjects& objects = currentRoomObjects();

  // advance projectiles, apply hits, and drop any that expired.
  for (auto& projectile : objects.projectiles)
  {
    if (projectile->isActive())
    {
      combat::advanceProjectile(*projectile, room, objects.enemies, player_);
    }
  }
  objects.projectiles.erase(
      std::remove_if(
          objects.projectiles.begin(), objects.projectiles.end(),
          [](const std::unique_ptr<Projectile>& p) { return !p->isActive(); }),
      objects.projectiles.end());

  // reap before the movement pass so every enemy iterated below is alive: a
  // projectile can drop one to 0 hp above, and advanceEnemy() doesn't
  // guard isAlive() itself.
  combat::reapDead(room, objects.enemies);

  // move enemies toward player or attack.
  for (auto& enemy : objects.enemies)
  {
    if (movement::advanceEnemy(*enemy, player_, room, goalMapCache_, services_))
    {
      combat::applyDamage(player_,
                          combat::meleeDamage(enemy->getAttackDamage()));
    }
  }

  // tick every entity's hit-flash once per frame, across all rooms: an enemy
  // hit just before the player leaves its room would otherwise freeze
  // mid-flash and show a stale tint on return. ticking after the damage pass
  // makes the frame a hit lands the first visible flash frame.
  player_.tickHitFlash();
  for (auto& [roomID, roomObjects] : levelData_.roomData)
  {
    for (auto& enemy : roomObjects.enemies)
    {
      enemy->tickHitFlash();
    }
  }
}

bool Game::playerMoved() const
{
  // room id as well as position.
  return player_.getPosition() != lastVisibilityPos_ ||
         currentRoomID_ != lastVisibilityRoomID_;
}

RenderState Game::buildRenderState() const
{
  RenderState state;

  const Room& room = currentRoom();
  for (int x = 0; x < Room::WIDTH; ++x)
  {
    for (int y = 0; y < Room::HEIGHT; ++y)
    {
      Coordinate position(x, y);
      TileView& tile = state.map.tiles[x][y];
      tile.symbol = room.getTileSymbol(position);

      if (room.isVisible(position))
      {
        tile.visibility = TileVisibility::Visible;
      }
      else if (room.isExplored(position))
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
  // the player dies.
  state.entity.player =
      EntityView{player_.getPosition(),
                 player_.isAlive() ? player_.getSymbol() : EntitySymbol{},
                 player_.hasHitFlash(), ColorPair::EntityHit};

  const RoomObjects& objects = currentRoomObjects();

  for (const auto& enemy : objects.enemies)
  {
    const Coordinate& position = enemy->getPosition();
    if (enemy->isAlive() && room.isVisible(position))
    {
      state.entity.enemies.push_back(EntityView{position, enemy->getSymbol(),
                                                enemy->hasHitFlash(),
                                                ColorPair::EntityHit});
    }
  }

  for (const auto& projectile : objects.projectiles)
  {
    const Coordinate& position = projectile->getPosition();
    if (projectile->isActive() && room.isVisible(position))
    {
      state.entity.projectiles.push_back(
          ProjectileView{position, projectile->getColor()});
    }
  }

  state.hud.playerHealth = player_.getHealth();
  state.hud.playerMaxHealth = player_.getMaxHealth();
  state.hud.roomIndex = currentRoomID_;
  state.hud.roomCount = levelData_.meta.roomCount;

  const Weapon& weapon = player_.getWeapon();
  state.hud.weapon =
      WeaponView{weapon.getName(), weapon.getDamage(), weapon.getSpeed(),
                 weapon.getRange(), weapon.getColor()};

  state.debug.playerPosition = player_.getPosition();
  state.debug.fps = currentFps_;

  return state;
}

void Game::render() { uiManager_.render(buildRenderState()); }
