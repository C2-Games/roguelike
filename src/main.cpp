#include "core/game.h"
#include "io/ui_manager.h"

int main()
{
  UIManager uiManager;
  Game game(uiManager);
  game.run();

  return 0;
}
