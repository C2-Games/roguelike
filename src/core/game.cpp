#include "core/game.h"

#include <ncurses.h>

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iostream>
#include <thread>

#include "entities/enemy.h"
#include "entities/enemy_registry.h"
#include "entities/move_context.h"
#include "render/ui.h"
#include "world/projectile.h"
#include "world/projectile_context.h"
#include "world/visibility.h"

Game::Game(int width, int height, int fps)
    : termWidth(width),
      termHeight(height),
      fps(fps),
      services(static_cast<std::mt19937::result_type>(std::time(nullptr))),
      player(Room::WIDTH / 2, Room::HEIGHT / 2),
      roomGraph(5, services),
      enemyRegistry(services),
      isRunning(true) {
  // generate enemy objects first..
  spawnEnemies();

  // add window overlays.
  // TODO: probably want to create an Enum for layer ordering.
  UI geom = computeUI(termHeight, termWidth);
  renderer.addLayer(
      1, std::make_unique<MapLayer>(geom.winHeight, geom.winWidth, geom.originY,
                                    geom.originX, roomGraph));
  renderer.addLayer(
      2, std::make_unique<EntityLayer>(geom.winHeight, geom.winWidth,
                                       geom.originY, geom.originX, roomGraph,
                                       player, enemies, projectiles));

  const int hud_margin = 2;
  renderer.addLayer(
      3, std::make_unique<HUDLayer>(termHeight, termWidth, hud_margin, player,
                                    roomGraph));

  // if debug build, add the debug window.
#ifndef NDEBUG
  renderer.addLayer(4, std::make_unique<DebugLayer>(termHeight, termWidth,
                                                    currentFps, player));
#endif
}

void Game::run() {
  printw("Roguelike Game Started! Use arrow keys to move. Press Q to quit.\n");
  printw("Press SPACE to begin...\n");

  int ch;
  do {
    ch = getch();
  } while (ch != ' ');

  const auto frame_duration = getDuration();

  while (isRunning && player.isAlive()) {
    // -------- Frame start --------
    auto start = std::chrono::high_resolution_clock::now();
    handleInput();
    update();
    render();

    // -------- Frame end --------
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    if (elapsed < frame_duration) {
      std::this_thread::sleep_for(frame_duration - elapsed);
    }

    // for debug window.
    std::chrono::duration<double, std::milli> totalFrame =
        std::chrono::high_resolution_clock::now() - start;

    currentFps = totalFrame.count() > 0.0 ? 1000.0 / totalFrame.count() : 0.0;
  }

  endwin();
  std::cout << "Game Over!\n";
}

void Game::handleInput() {
  int ch = getch();
  Coordinate newPlayerPos = player.getPosition();

  switch (ch) {
    case KEY_RESIZE:  // in case of a terminal resize.
      getmaxyx(stdscr, termHeight, termWidth);
      renderer.resizeAll(termHeight, termWidth);
      return;
    case KEY_UP:
    case 'w':  // Up
      newPlayerPos.y -= 1;
      player.setLastDirection(Coordinate(0, -1));
      break;
    case KEY_DOWN:
    case 's':  // Down
      newPlayerPos.y += 1;
      player.setLastDirection(Coordinate(0, 1));
      break;
    case KEY_LEFT:
    case 'a':  // Left
      newPlayerPos.x -= 1;
      player.setLastDirection(Coordinate(-1, 0));
      break;
    case KEY_RIGHT:
    case 'd':  // Right
      newPlayerPos.x += 1;
      player.setLastDirection(Coordinate(1, 0));
      break;
    case ' ': {
      // "fire" a projectile in the player's last-faced direction.
      Coordinate dir = player.getLastDirection();  // acts as an offset.
      Coordinate spawnPos = player.getPosition() + dir;
      const Weapon& weapon = player.getWeapon();

      projectiles.push_back(std::make_unique<Projectile>(
          spawnPos, dir, weapon.getDamage(), weapon.getSpeed(),
          weapon.getRange(), weapon.getColor()));
      return;
    }
    case 'q':
    case 'Q':
      isRunning = false;
      break;
    default:
      mvprintw(2, 0, "Invalid key\n");
      break;
  }
  if (newPlayerPos.x >= 0 && newPlayerPos.x < Room::WIDTH &&
      newPlayerPos.y >= 0 && newPlayerPos.y < Room::HEIGHT &&
      roomGraph.getCurrentRoom()
          .tiles[newPlayerPos.x][newPlayerPos.y]
          .isWalkable()) {
    // Check for a linked door before applying normal movement.
    if (roomGraph.getCurrentRoom()
            .tiles[newPlayerPos.x][newPlayerPos.y]
            .getType() == TileType::Door) {
      const DoorConnection* conn = roomGraph.getDoorConnection(
          roomGraph.getCurrentRoomID(), newPlayerPos);
      if (conn) {
        transitionRoom(*conn);
        return;
      }
    }
    player.moveTo(newPlayerPos);
  }
}

void Game::update() {
  // Recompute FoV visibility for the current room before anything else
  // runs this frame.
  visibility::update(roomGraph.getCurrentRoom(), player.getPosition(),
                     player.getFOV());

  const Coordinate playerPos = player.getPosition();
  const Room& currentRoom = roomGraph.getCurrentRoom();

  // Build the per-frame contexts once and reuse across every enemy/projectile.
  MoveContext moveCtx{playerPos, currentRoom, goalMapCache, enemies, services};

  ProjectileContext projCtx{
      currentRoom, [&](Coordinate pos, int damage) -> bool {
        // Try to damage the first live enemy sitting on `pos`.
        auto hit =
            std::find_if(enemies.begin(), enemies.end(),
                         [&pos](const std::unique_ptr<Enemy>& e) {
                           return e->isAlive() && e->getPosition() == pos;
                         });
        if (hit == enemies.end()) return false;
        (*hit)->takeDamage(damage);
        return true;
      }};

  // Move enemies toward player.
  for (auto& enemy : enemies) {
    if (enemy->isAlive()) {
      enemy->moveTowardPlayer(moveCtx);

      // Check collision with player
      if (enemy->getPosition() == playerPos) {
        player.takeDamage(enemy->getAttackDamage());
      }
    }
  }

  // Advance projectiles, apply hits, and drop any that expired.
  for (auto& projectile : projectiles) {
    if (projectile->isActive()) {
      projectile->update(projCtx);
    }
  }
  projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(),
                                   [](const std::unique_ptr<Projectile>& p) {
                                     return !p->isActive();
                                   }),
                    projectiles.end());

  // Reap dead enemies at end-of-frame. Safe here because all per-frame
  // iterations over `enemies` (movement, player-collision, projectile
  // hit-tests) have completed above.
  EnemyRegistry::reap(enemies);
}

void Game::render() { renderer.compose(); };

void Game::spawnEnemies() {
  // Load enemies for whichever room the player starts in (RoomGraph's current
  // cursor). This lets a future SaveSystem restore the starting room by
  // setting the cursor before construction, without touching this call site.
  const int startRoomID = roomGraph.getCurrentRoomID();
  enemyRegistry.loadForRoom(startRoomID, roomGraph.getRoom(startRoomID),
                            enemies);
}

void Game::transitionRoom(const DoorConnection& conn) {
  // Persist current room's enemies and load the destination room's.
  enemyRegistry.transitionActive(roomGraph.getCurrentRoomID(), conn.destRoomID,
                                 roomGraph.getRoom(conn.destRoomID), enemies);

  // Switch the active room.
  roomGraph.setCurrentRoomID(conn.destRoomID);

  // Place the player one tile inward from the destination door so they
  // don't immediately re-trigger the door on the next input.
  Coordinate dest = conn.destDoorPos;
  Coordinate inward = dest;
  if (dest.x == 0)
    inward.x = 1;
  else if (dest.x == Room::WIDTH - 1)
    inward.x = Room::WIDTH - 2;
  else if (dest.y == 0)
    inward.y = 1;
  else if (dest.y == Room::HEIGHT - 1)
    inward.y = Room::HEIGHT - 2;

  player.moveTo(inward);
}
