#include "core/game.h"

#include <ncurses.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

#include "entities/enemy.h"
#include "entities/move_context.h"
#include "render/window_position.h"
#include "world/objects/projectile.h"
#include "world/objects/projectile_context.h"

namespace
{
// Fixed until a real seed-input/new-game flow exists: every launch should
// reproduce the same room/enemy layout for a given level config.
constexpr std::mt19937::result_type kDefaultSeed = 80085;
}  // namespace

Game::Game(int width, int height, int fps)
    : termWidth_(width),
      termHeight_(height),
      fps_(fps),
      services_(kDefaultSeed),
      player_(Room::WIDTH / 2, Room::HEIGHT / 2),
      enemyCatalog_("assets/enemies"),
      level_("assets/levels/level_1", services_, enemyCatalog_),
      isRunning_(true)
{
  // add window overlays.
  WindowPosition geom = centerWindow(termHeight_, termWidth_);
  renderer_.addLayer(
      1, std::make_unique<MapLayer>(geom.winHeight, geom.winWidth, geom.originY,
                                    geom.originX, level_));
  renderer_.addLayer(2, std::make_unique<EntityLayer>(
                            geom.winHeight, geom.winWidth, geom.originY,
                            geom.originX, level_, player_, projectiles_));

  const int hud_margin = 2;
  renderer_.addLayer(
      3, std::make_unique<HUDLayer>(termHeight_, termWidth_, hud_margin,
                                    player_, level_));

  // if debug build, add the debug window.
#ifndef NDEBUG
  renderer_.addLayer(RenderLayer::Debug,
                     std::make_unique<DebugLayer>(termHeight_, termWidth_,
                                                  currentFps_, player_));
#endif
}

void Game::run()
{
  printw("Roguelike Game Started! Use arrow keys to move. Press Q to quit.\n");
  printw("Press SPACE to begin...\n");

  int ch;
  do
  {
    ch = getch();
  } while (ch != ' ');

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

  endwin();
  std::cout << "Game Over!\n";
}

void Game::handleInput()
{
  int ch = getch();
  Coordinate newPlayerPos = player_.getPosition();

  switch (ch)
  {
    case KEY_RESIZE:  // in case of a terminal resize.
      getmaxyx(stdscr, termHeight_, termWidth_);
      renderer_.resizeAll(termHeight_, termWidth_);
      return;
    case KEY_UP:
    case 'w':  // Up
      newPlayerPos.y -= 1;
      player_.setLastDirection(Coordinate(0, -1));
      break;
    case KEY_DOWN:
    case 's':  // Down
      newPlayerPos.y += 1;
      player_.setLastDirection(Coordinate(0, 1));
      break;
    case KEY_LEFT:
    case 'a':  // Left
      newPlayerPos.x -= 1;
      player_.setLastDirection(Coordinate(-1, 0));
      break;
    case KEY_RIGHT:
    case 'd':  // Right
      newPlayerPos.x += 1;
      player_.setLastDirection(Coordinate(1, 0));
      break;
    case ' ':
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
    case 'q':
    case 'Q':
      isRunning_ = false;
      break;
    default:
      mvprintw(2, 0, "Invalid key\n");
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
      if (conn)
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
  if (playerMoved())
  {
    level_.getCurrentRoom().updateVisibility(player_.getPosition(),
                                             player_.getFOV());
    lastVisibilityPos_ = player_.getPosition();
    lastVisibilityRoomID_ = level_.getCurrentRoomID();
  }

  const Coordinate playerPos = player_.getPosition();
  Room& currentRoom = level_.getCurrentRoom();

  // Reap before the movement pass so every enemy iterated below is alive.
  // Must precede MoveContext, which holds a reference into this vector.
  RoomEnemyState::reap(currentRoom.enemies());

  // Build the per-frame contexts once and reuse across every enemy/projectile.
  MoveContext moveCtx{playerPos, currentRoom, goalMapCache_,
                      currentRoom.enemies(), services_};

  ProjectileContext projCtx{
      currentRoom, [&](Coordinate pos, int damage) -> bool {
        // Try to damage the first live enemy sitting on `pos`.
        auto& enemies = currentRoom.enemies();
        auto hit =
            std::find_if(enemies.begin(), enemies.end(),
                         [&pos](const std::unique_ptr<Enemy>& e) {
                           return e->isAlive() && e->getPosition() == pos;
                         });
        if (hit == enemies.end())
        {
          return false;
        }
        (*hit)->takeDamage(damage);
        return true;
      }};

  // Move enemies toward player.
  for (auto& enemy : currentRoom.enemies())
  {
    enemy->moveTowardPlayer(moveCtx);

    // Check collision with player
    if (enemy->getPosition() == playerPos)
    {
      player_.takeDamage(enemy->getAttackDamage());
    }
  }

  // Advance projectiles, apply hits, and drop any that expired.
  for (auto& projectile : projectiles_)
  {
    if (projectile->isActive())
    {
      projectile->update(projCtx);
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

void Game::render() { renderer_.compose(); };
