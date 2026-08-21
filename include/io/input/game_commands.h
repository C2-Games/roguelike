#ifndef GAME_COMMANDS_H
#define GAME_COMMANDS_H

enum class GameCommand
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
