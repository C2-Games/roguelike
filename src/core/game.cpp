#include "core/game.h"

#include <algorithm>
#include <chrono>
#include <thread>

#include "core/frame_state.h"
#include "core/render_state_builder.h"
#include "entities/enemy.h"
#include "entities/entity.h"
#include "io/input/game_commands.h"
#include "io/ui_manager.h"
#include "world/objects/projectile.h"

namespace
{
// Fixed until a real seed-input/new-game flow exists: every launch should
// reproduce the same room/enemy layout for a given level config.
constexpr std::mt19937::result_type kDefaultSeed = 80085;
// Duration, in frames, that the player's hit-flash stays visible.
constexpr int HIT_FLASH_FRAMES = 8;
}  // namespace

Game::Game(UIManager& uiManager, int fps)
    : fps_(fps),
      services_(kDefaultSeed),
      player_(Coordinate(Room::WIDTH / 2, Room::HEIGHT / 2)),
      enemyCatalog_("assets/enemies"),
      level_("assets/levels/level_1", services_, enemyCatalog_),
      isRunning_(true),
      uiManager_(uiManager)
{}

void Game::run()
{
  UIManager::showStartScreen();

  const auto frame_duration = getDuration();

  while (isRunning_ && player_.isAlive())
  {
    // -------- Frame start --------
    auto start = std::chrono::high_resolution_clock::now();
    handleInput();
    update();
    render();

    // -------- Frame end --------
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    if (elapsed < frame_duration)
    {
      std::this_thread::sleep_for(frame_duration - elapsed);
    }

    // for debug window.
    std::chrono::duration<double, std::milli> totalFrame =
        std::chrono::high_resolution_clock::now() - start;

    currentFps_ = totalFrame.count() > 0.0 ? 1000.0 / totalFrame.count() : 0.0;
  }

  UIManager::showGameOver();
}

void Game::handleInput()
{
  GameCommand command = uiManager_.pollInput();
  Coordinate newPlayerPos = player_.getPosition();

  switch (command)
  {
    case GameCommand::Resize:
      // unreachable: UIManager::pollInput() already resolves resizes to
      // None before Game sees them. kept only so the switch stays
      // exhaustive over GameCommand.
      break;
    case GameCommand::MoveUp:
      newPlayerPos.y -= 1;
      player_.setLastDirection(Coordinate(0, -1));
      break;
    case GameCommand::MoveDown:
      newPlayerPos.y += 1;
      player_.setLastDirection(Coordinate(0, 1));
      break;
    case GameCommand::MoveLeft:
      newPlayerPos.x -= 1;
      player_.setLastDirection(Coordinate(-1, 0));
      break;
    case GameCommand::MoveRight:
      newPlayerPos.x += 1;
      player_.setLastDirection(Coordinate(1, 0));
      break;
    case GameCommand::Attack:
    {
      // "fire" a projectile in the player's last-faced direction.
      Coordinate dir = player_.getLastDirection();  // acts as an offset.
      Coordinate spawnPos = player_.getPosition() + dir;
      const Weapon& weapon = player_.getWeapon();

      projectiles_.push_back(std::make_unique<Projectile>(
          spawnPos, dir, weapon.getDamage(), weapon.getSpeed(),
          weapon.getRange(), weapon.getColor()));
      return;
    }
    case GameCommand::Quit:
      isRunning_ = false;
      break;
    case GameCommand::None:
      break;
  }
  if (newPlayerPos.x >= 0 && newPlayerPos.x < Room::WIDTH &&
      newPlayerPos.y >= 0 && newPlayerPos.y < Room::HEIGHT &&
      level_.getCurrentRoom()
          .tiles[newPlayerPos.x][newPlayerPos.y]
          .isWalkable())
  {
    // Check for a linked door before applying normal movement.
    if (level_.getCurrentRoom()
            .tiles[newPlayerPos.x][newPlayerPos.y]
            .getType() == TileType::Door)
    {
      const DoorConnection* conn =
          level_.getDoorConnection(level_.getCurrentRoomID(), newPlayerPos);
      if (conn != nullptr)
      {
        player_.moveTo(level_.transitionRoom(*conn));
        return;
      }
    }
    player_.moveTo(newPlayerPos);
  }
}

void Game::update()
{
  // Recompute FoV visibility for the current room before anything else runs
  // this frame, but only when it can have changed.
  bool sameRoomAndShape = level_.getCurrentRoomID() == lastVisibilityRoomID_ &&
                          player_.getFOV() == *lastVisibilityFov_;
  if (playerMoved() || !sameRoomAndShape)
  {
    Room& room = level_.getCurrentRoom();
    if (sameRoomAndShape)
    {
      room.updateVisibility(lastVisibilityPos_, player_.getPosition(),
                            player_.getFOV());
    }
    else
    {
      room.updateVisibility(player_.getPosition(), player_.getFOV());
    }
    lastVisibilityPos_ = player_.getPosition();
    lastVisibilityRoomID_ = level_.getCurrentRoomID();
    lastVisibilityFov_ = player_.getFOV().clone();
  }

  Room& currentRoom = level_.getCurrentRoom();

  // Reap before the movement pass so every enemy iterated below is alive.
  RoomEnemyState::reap(currentRoom.enemies());

  // Build the per-frame context once and reuse across every enemy/projectile.
  FrameState frame{player_, currentRoom};

  // Move enemies toward player or attack
  for (auto& enemy : currentRoom.enemies())
  {
    if (enemy->moveTowardPlayer(frame, goalMapCache_, services_))
    {
      player_.takeDamage(enemy->getAttackDamage());
      playerHitFlashFramesRemaining_ = HIT_FLASH_FRAMES;
    }
  }
  if (playerHitFlashFramesRemaining_ > 0)
  {
    --playerHitFlashFramesRemaining_;
  }

  // Advance projectiles, apply hits, and drop any that expired.
  for (auto& projectile : projectiles_)
  {
    if (projectile->isActive())
    {
      bool hitEntity = projectile->update(frame);
      if (hitEntity)
      {
        if (Entity* target =
                currentRoom.entityAt(projectile->getPosition(), player_))
        {
          target->takeDamage(projectile->getDamage());
        }
      }
    }
  }
  projectiles_.erase(std::remove_if(projectiles_.begin(), projectiles_.end(),
                                    [](const std::unique_ptr<Projectile>& p) {
                                      return !p->isActive();
                                    }),
                     projectiles_.end());
}

bool Game::playerMoved() const
{
  // room id as well as position
  return player_.getPosition() != lastVisibilityPos_ ||
         level_.getCurrentRoomID() != lastVisibilityRoomID_;
}

void Game::render()
{
  uiManager_.render(
      render_state_builder::build(player_, level_, projectiles_, currentFps_,
                                  playerHitFlashFramesRemaining_ > 0));
}
