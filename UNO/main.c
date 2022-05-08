#include "UNO.h"

int main()
{
  Game *game;
  game = (Game *)malloc(sizeof(Game));

  Start(game);

  // loop del gioco
  while (!is_over(game))
  {
    display(game);
    get_move(game);
    update(game);
  }

  return 0;
}
