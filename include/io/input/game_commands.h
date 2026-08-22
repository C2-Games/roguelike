#ifndef GAME_COMMANDS_H
#define GAME_COMMANDS_H

#include <cstdint>

enum class GameCommand : std::uint8_t
{
  None,
  MoveUp,
  MoveDown,
  MoveLeft,
  MoveRight,
  Attack,
  Quit,
  // consumed by UIManager::pollInput(); Game never observes this value.
  Resize,
};

#endif
