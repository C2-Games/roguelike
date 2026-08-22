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
#include "io/output/render_state.h"
#include "io/output/window_position.h"

namespace
{
// gap, in rows, between the HUD band and the map layer's top border.
constexpr int HUD_MARGIN = 2;

// composes one layer onto stdscr if it's enabled.
template <typename Layer, typename Packet>
void composeLayer(Layer& layer, const Packet& packet)
{
  if (!layer.isEnabled()) return;
  layer.doUpdate();
  layer.doRender(packet);
  overlay(layer.getWindow(), stdscr);
}
}  // namespace

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
  mapLayer_ = std::make_unique<MapLayer>(geom.winHeight, geom.winWidth,
                                         geom.originY, geom.originX);
  entityLayer_ = std::make_unique<EntityLayer>(geom.winHeight, geom.winWidth,
                                               geom.originY, geom.originX);

  hudLayer_ = std::make_unique<HUDLayer>(termHeight_, termWidth_, HUD_MARGIN);

  // if debug build, add the debug window.
#ifndef NDEBUG
  debugLayer_ = std::make_unique<DebugLayer>(termHeight_, termWidth_);
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
    mapLayer_->onResize(termHeight_, termWidth_);
    entityLayer_->onResize(termHeight_, termWidth_);
    hudLayer_->onResize(termHeight_, termWidth_);
#ifndef NDEBUG
    if (debugLayer_) debugLayer_->onResize(termHeight_, termWidth_);
#endif
    return GameCommand::None;
  }
  return command;
}

void UIManager::render(const RenderState& state)
{
  erase();

  composeLayer(*mapLayer_, state.map);
  composeLayer(*entityLayer_, state.entity);
  composeLayer(*hudLayer_, state.hud);
#ifndef NDEBUG
  if (debugLayer_) composeLayer(*debugLayer_, state.debug);
#endif

  refresh();
}

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
