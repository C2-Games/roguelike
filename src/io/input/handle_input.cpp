#include "io/input/handle_input.h"

#include <ncurses.h>

namespace input
{

GameCommand pollInput()
{
  int ch = getch();

  switch (ch)
  {
    case KEY_RESIZE:
      return GameCommand::Resize;
    case KEY_UP:
    case 'w':
      return GameCommand::MoveUp;
    case KEY_DOWN:
    case 's':
      return GameCommand::MoveDown;
    case KEY_LEFT:
    case 'a':
      return GameCommand::MoveLeft;
    case KEY_RIGHT:
    case 'd':
      return GameCommand::MoveRight;
    case ' ':
      return GameCommand::Attack;
    case 'q':
    case 'Q':
      return GameCommand::Quit;
    default:
      return GameCommand::None;
  }
}

}  // namespace input
