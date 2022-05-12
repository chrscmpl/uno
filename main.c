#include "UNO.h"

int main()
{
    do
    {
        Game *game;
        game = (Game *)malloc(sizeof(Game));

        start(game);

        // loop del gioco
        while (!game->GameOver)
        {
            display(game);
            get_move(game);
            update(game);
        }

        end_game(game);

    } while (play_again());

    return 0;
}
