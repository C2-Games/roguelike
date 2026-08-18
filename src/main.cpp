#include <ncurses.h>

#include <clocale>

#include "core/game.h"

int main()
{
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
