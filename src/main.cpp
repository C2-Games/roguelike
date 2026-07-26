#include <ncurses.h>

#include <clocale>
#include <cstdlib>
#include <ctime>

#include "core/game.h"

int main() {
  // Seed the random number generator once at startup so room shapes, door
  // layouts, and enemy positions differ on every run.
  srand(static_cast<unsigned int>(time(nullptr)));

  // Enable locale-aware (UTF-8) input/output for ncursesw.
  setlocale(LC_ALL, "");

  // Initialize ncurses
  initscr();
  initColors();
  cbreak();
  noecho();
  nodelay(stdscr, TRUE);

  // include multiple special keys including keypad
  keypad(stdscr, TRUE);

  // Remove cursor from screen
  curs_set(0);

  // get terminal size
  int termHeight, termWidth;
  getmaxyx(stdscr, termHeight, termWidth);

  // start game.
  Game game(termWidth, termHeight);
  game.run();

  return 0;
}
