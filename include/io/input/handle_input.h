#ifndef HANDLE_INPUT_H
#define HANDLE_INPUT_H

#include "io/input/game_commands.h"

namespace input
{

// reads one ncurses key press and maps it to a GameCommand, or
// GameCommand::None if the key has no mapping.
GameCommand pollInput();

}  // namespace input

#endif
