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
  // handled inline by Game for now; a future UIManager will intercept this
  // before Game ever sees it.
  Resize,
};

#endif
