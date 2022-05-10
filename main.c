#include "UNO.h"

int main()
{
    do {
        Game* game;
        game = (Game*)malloc(sizeof(Game));

        Start(game);

        // loop del gioco
        while (!GameOver)
        {
            display(game);
            get_move(game);
            update(game);
        }
    } while (play_again());

  return 0;
}
