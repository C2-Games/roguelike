#include "io/ui_manager.h"

#include <ncurses.h>

#include <clocale>
#include <iostream>

#include "io/input/handle_input.h"
#include "io/output/colors.h"
#include "io/output/layers/debug_layer.h"
#include "io/output/layers/entity_layer.h"
#include "io/output/layers/hud_layer.h"
#include "io/output/layers/map_layer.h"
#include "io/output/window_position.h"

UIManager::UIManager()
{
  // enable locale-aware (utf-8) input/output for ncursesw.
  setlocale(LC_ALL, "");

  initscr();
  initColors();
  cbreak();
  noecho();
  nodelay(stdscr, TRUE);

  // include multiple special keys including keypad.
  keypad(stdscr, TRUE);

  // remove cursor from screen.
  curs_set(0);

  getmaxyx(stdscr, termHeight_, termWidth_);

  // add window overlays.
  WindowPosition geom = centerWindow(termHeight_, termWidth_);
  renderer_.addLayer(RenderLayer::Map,
                     std::make_unique<MapLayer>(geom.winHeight, geom.winWidth,
                                                geom.originY, geom.originX));
  renderer_.addLayer(RenderLayer::Entity, std::make_unique<EntityLayer>(
                                              geom.winHeight, geom.winWidth,
                                              geom.originY, geom.originX));

  const int hudMargin = 2;
  renderer_.addLayer(RenderLayer::HUD, std::make_unique<HUDLayer>(
                                           termHeight_, termWidth_, hudMargin));

  // if debug build, add the debug window.
#ifndef NDEBUG
  renderer_.addLayer(RenderLayer::Debug,
                     std::make_unique<DebugLayer>(termHeight_, termWidth_));
#endif
}

UIManager::~UIManager()
{
  if (!isendwin())
  {
    endwin();
  }
}

GameCommand UIManager::pollInput()
{
  GameCommand command = input::pollInput();
  if (command == GameCommand::Resize)
  {
    getmaxyx(stdscr, termHeight_, termWidth_);
    renderer_.resizeAll(termHeight_, termWidth_);
    return GameCommand::None;
  }
  return command;
}

void UIManager::render(const RenderState& state) { renderer_.compose(state); }

void UIManager::showStartScreen()
{
  printw("Roguelike Game Started! Use arrow keys to move. Press Q to quit.\n");
  printw("Press SPACE to begin...\n");

  int ch;
  do
  {
    ch = getch();
  } while (ch != ' ');
}

void UIManager::showGameOver()
{
  endwin();
  std::cout << "Game Over!\n";
}
