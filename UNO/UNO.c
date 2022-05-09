#include "UNO.h"

void Start(Game *game)
{
    srand(time(0)); // per valori randomici

    // inizializazione mazzo principale
    game->Deck = (struct card *)malloc(sizeof(struct card) * SIZE_DECK);
    game->SzDeck = SIZE_DECK;
    shuffle(game);

    // inizializzazione mani dei giocatori
    game->SzPlayers = get_players();
    init_players(game);

    // inizializzazione variabili
    game->CurrentPlayer = 0;
    game->Rotation = true; // true = sinistra; false = destra;
    game->SzDeck--;
    game->DiscardDeck = *(game->Deck + game->SzDeck);
    game->Plus = 0;
    game->FirstTurn = true;
}

// richiede dalla tastiera l'inserimento del numero di giocatori
int get_players()
{
    int p = 0;
    printf("inserire il numero di giocatori: ");
    while (p < 2 || p > 4)
    {
        scanf("%d", &p);
        clean_stdin();
        if (p < 2 || p > 4)
            printf("numero di giocatori non valido, inserire un numero di giocatori compreso tra 2 e 4: ");
    }
    system(clear);
    return p;
}

// inizializza il mazzo principale
void shuffle(Game *game)
{

    for (int i = 0; i < game->SzDeck; i++) // inizializza il mazzo
        (game->Deck + i)->color = na;

    for (int c = 1; c < 5; c++) // per ogni colore
    {
        // carte numero
        for (int i = 0; i < 10; i++)
        {
            for (int j = 0; j == 0 || (j == 1 && i != 0); j++) // c'è solo uno zero per colore
            {
                struct card *pos = find_empty_space(game);
                pos->color = (enum col)c;
                pos->front[0] = i + 48; // ascii
                pos->front[1] = '\0';
            }
        }

        // carte stop
        for (int i = 0; i < 2; i++)
        {
            struct card *pos = find_empty_space(game);
            pos->color = (enum col)c;
            strcpy(pos->front, "S");
        }

        // carte reverse
        for (int i = 0; i < 2; i++)
        {
            struct card *pos = find_empty_space(game);
            pos->color = (enum col)c;
            strcpy(pos->front, "R");
        }

        // carte +2
        for (int i = 0; i < 2; i++)
        {
            struct card *pos = find_empty_space(game);
            pos->color = (enum col)c;
            strcpy(pos->front, "+2");
        }
    }

    // carte nere choose
    for (int i = 0; i < 4; i++)
    {
        struct card *pos = find_empty_space(game);
        pos->color = n;
        strcpy(pos->front, "C");
    }

    // carte nere +4
    for (int i = 0; i < 4; i++)
    {
        struct card *pos = find_empty_space(game);
        pos->color = n;
        strcpy(pos->front, "+4");
    }
}

// cerca spazi vuoti nel mazzo
struct card *find_empty_space(Game *game)
{
    struct card *space;
    do
    {
        space = (game->Deck + (rand() % game->SzDeck));
    } while (space->color != na);
    return space;
}

// inizializza i mazzi dei giocatori
void init_players(Game *game)
{
    // inizializza l'array di mazzi
    game->Players = (struct card **)malloc(sizeof(struct card *) * game->SzPlayers);

    // inizializza le singole mani
    for (int i = 0; i < game->SzPlayers; i++)
        *(game->Players + i) = (struct card *)malloc(sizeof(struct card) * game->SzDeck); // alloca lo spazio necessario

    for (int i = 0; i < game->SzPlayers; i++) // pesca carte dal mazzo
        for (int j = 0; j < STARTING_HAND_SIZE; j++)
        {
            game->SzDeck--;
            ((*(game->Players + i)) + j)->color = (game->Deck + game->SzDeck)->color;
            strcpy((*(game->Players + i) + j)->front, (game->Deck + game->SzDeck)->front);
        }

    // inizializza le dimensioni delle mani
    game->SzHands = (int *)malloc(sizeof(int) * game->SzPlayers);
    for (int i = 0; i < game->SzPlayers; i++)
        *(game->SzHands + i) = STARTING_HAND_SIZE;
}

bool is_over(Game *game)
{
    // finito il gioco game e le sue variabili vengono deallocate e game impostato a NULL che vale 0
    return !game;
}
// stampa a video la mano del giocatore corrente e la carta in cima al mazzo discard
void display(Game *game)
{
    system(clear);

    printf("Turno del Giocatore %d\n\n", game->CurrentPlayer + 1); // mostra il numero del giocatore corrente

    // mostra il numero di carte nelle mani degli avversari, nell'ordine della rotazione
    for (int i = game->Rotation ? 0 : (game->SzPlayers - 1); (game->Rotation && (i < game->SzPlayers)) || (!game->Rotation && (i >= 0));)
    {
        if (i != game->CurrentPlayer)
            printf("il giocatore %d ha %d carte nella sua mano\n", i + 1, *(game->SzHands + i));
        if (game->Rotation)
            i++;
        else
            i--;
    }

    // mostra il verso di rotazione
    printf("\nLa rotazione e' attualmente in senso %s\n\n\n", game->Rotation ? "orario" : "antiorario");

    for (int i = 0; i < (*(game->SzHands + game->CurrentPlayer)); i++) // aggiusta il mazzo discard più o meno
        printf("\t");                                                  // al centro rispetto alla mano

    // la carta in cima al mazzo discard
    printf("%s\n\n\n\n\n\t", displayed_card(&game->DiscardDeck));

    for (int i = 0; i < (*(game->SzHands + game->CurrentPlayer)); i++) // mostra la mano del giocatore
        printf("%s\t\t", displayed_card(*(game->Players + game->CurrentPlayer) + i));

    printf("%s\n\n\n\n", RESET);
}

// converte una carta in una stringa che ha come prefisso la sequenza di escape
// corrispondente al suo colore
const char *displayed_card(struct card *c)
{
    // imposta il colore
    char color[6] = RESET;
    switch (c->color)
    {
    case r:
        strcpy(color, RED);
        break;
    case g:
        strcpy(color, GREEN);
        break;
    case b:
        strcpy(color, BLUE);
        break;
    case y:
        strcpy(color, YELLOW);
        break;
    case n:
        strcpy(color, RESET);
    }

    // per le carte speciali
    char *face = NULL;
    switch (c->front[0])
    {
    case 'S':
        face = (char *)malloc(sizeof(char) * 5);
        strcpy(face, "Stop");
        break;
    case 'R':
        face = (char *)malloc(sizeof(char) * 8);
        strcpy(face, "Reverse");
        break;
    case 'C':
        face = (char *)malloc(sizeof(char) * 7);
        strcpy(face, "Choose");
    }

    // compone la stringa
    static char colored_card[13];
    strcpy(colored_card, color);
    strcat(colored_card, (face ? face : c->front));

    free(face);
    return colored_card;
}

void lowercase(char *str)
{
    // converte tutti i caratteri maiuscoli e minuscoli per le
    while (*str)
    {
        if (*str >= 'A' && *str <= 'Z')
            *str += 32;
        str++;
    }
}

void get_move(Game *game)
{
    // +2 e +4
    if (game->Plus) {
        plus(game);
        return;
    }

    // in caso di nessuna mossa disponibile si pesca
    if (check_draw(game))
    {
        // game->Move = 'd';
        game->Move = '0';
        return;
    }

    printf("Seleziona la carta che intendi giocare,\noppure digita 'aiuto' per consultare le regole: ");

    char move[20];

    while (true)
    {
        struct card chosen;
        chosen.front[0] = '\0'; // inizializzazione variabile
        chosen.color = na;

        strcpy(move, read_words());

        short spazi = 0;
        char *temp = move; // converte tutti i caratteri maiuscoli e minuscoli per le
        while (*temp)      // future comparazioni e controlla il numero di spazi
        {
            if (*temp == ' ')
                spazi++;
            temp++;
        }

        // regolamento
        if (!strcmp(move, "aiuto"))
        {
            // game->Move = 'h';
            game->Move = '0';
            return;
        }

        // ricorda di dire Uno!
        if (strcmp(move, "uno") && *(game->SzHands + game->CurrentPlayer) == 1)
        {
            // game->Move = 'u';
            game->Move = '0';
            return;
        }

        if (spazi == 1 || !strcmp(move, "choose") || !strcmp(move, "+4")) // poichè per le nere non serve specificare il colore non ci sono spazi
        {

            char *token = strtok(move, " "); // prende la prima parola, corrispondente alla faccia

            // carte numero
            if (*token >= '0' && *token <= '9' && *(token + 1) == '\0')
                strcpy(chosen.front, token);
            // carte +2 o +4
            else if (*token == '+' && (*(token + 1) == '2' || *(token + 1) == '4') && *(token + 2) == '\0')
            {
                strcpy(chosen.front, token);
                if (chosen.front[1] == '4')
                    chosen.color = n;
            }
            // carte speciali
            else if (!strcmp(token, "reverse"))
                strcpy(chosen.front, "R");
            else if (!strcmp(token, "stop"))
                strcpy(chosen.front, "S");
            else if (!strcmp(token, "choose"))
            {
                strcpy(chosen.front, "C");
                chosen.color = n;
            }

            // prende la seconda parola, corrispondente al colore
            if (chosen.color == na)
            {
                token = strtok(NULL, " ");

                if (!strcmp(token, "rosso"))
                    chosen.color = r;
                else if (!strcmp(token, "verde"))
                    chosen.color = g;
                else if (!strcmp(token, "blu"))
                    chosen.color = b;
                else if (!strcmp(token, "giallo"))
                    chosen.color = y;
            }

            // controlla la validità della mossa
            if (chosen.front && chosen.color != na)
            {
                // si assicura che nel caso in cui ci sia un +2 o +4 e il giocatore possa rispondere lo faccia
                if (((!strcmp(game->DiscardDeck.front, "+2") || !strcmp(game->DiscardDeck.front, "+4")) && strcmp(chosen.front, game->DiscardDeck.front)) && !game->FirstTurn)
                {
                    printf("Gioca il tuo %s: ", (strcmp(chosen.front, "+2") ? "+4" : "+2"));
                }
                else
                {
                    // confronto con quella in cima al mazzo discard
                    if (strcmp(chosen.front, game->DiscardDeck.front) && chosen.color != game->DiscardDeck.color && chosen.color != n && game->DiscardDeck.color != n)
                    {
                        printf("La carta non e' compatibile con quella in cima al mazzo Discard, selezionane un'altra: ");
                    }
                    else
                    {
                        // confronto con quelle nella mano del giocatore
                        for (int i = 0; i < *(game->SzHands + game->CurrentPlayer); i++)
                        {
                            struct card temp = *(*(game->Players + game->CurrentPlayer) + i);
                            if (!strcmp(temp.front, chosen.front) && chosen.color == temp.color)
                            {
                                game->FirstTurn = false;
                                game->Move = i + 48;
                                return;
                            }
                        }

                        printf("Non hai quella carta, selezionane un'altra: ");
                    }
                }
            }
            else
            {
                printf("Seleziona una mossa valida: ");
            }
        }
        else
        {
            printf("Seleziona una mossa valida: ");
        }
    }
}

const char* read_words() {
    static char move[20];

    scanf("%20[^\n]", move); // per leggere l'intera riga senza fermarsi agli spazi
    clean_stdin();

    lowercase(move);

    return move;
}

void update(Game *game)
{
    struct card *player = *(game->Players + game->CurrentPlayer); // leggibilità
    int *szHand = (game->SzHands + game->CurrentPlayer);
    int played = game->Move - 48;
    short stop = 0;
    // help e pescate
    switch (game->Move)
    {
    case 'h':
        help();
        return;
    case 'd':
    case 'u':
    case '+':
        return;
    }

    // aggiorna il mazzo discard
    game->DiscardDeck = *(player + played);

    // effetti delle carte
    switch ((player + played)->front[0])
    {
    case 'S':
        stop = 1;
        break;
    case 'R':
        game->Rotation = !game->Rotation;
        break;
    case 'C':
        game->DiscardDeck.color = (enum col)choose_color();
        break;
    case '+':
        if ((player + played)->front[1] == '4')
            game->DiscardDeck.color = (enum col)choose_color();
        game->Plus += (player + played)->front[1] - 48;
    }

    // aggiorna la mano
    for (int i = played; i < *szHand - 1; i++)
        *(player + i) = *(player + i + 1);
    (*szHand)--;

    // controlla vittoria
    if (!(*szHand))
        player = NULL;

    // passa il turno
    if (game->Rotation)
    {
        game->CurrentPlayer += 1 + stop;
        if (game->CurrentPlayer >= game->SzPlayers)
            game->CurrentPlayer -= game->SzPlayers;
    }
    else
    {
        game->CurrentPlayer -= 1 + stop;
        if (game->CurrentPlayer < 0)
            game->CurrentPlayer += game->SzPlayers;
    }
    fflush(stdin);
    game->Move = ' ';
}

int choose_color()
{
    printf("Scegli il colore della carta giocata: ");
    char *color = (char *)malloc(sizeof(char) * 10);

    int res = 0;
    while (res == 0)
    {
        scanf("%s", color);
        clean_stdin();
        lowercase(color);
        if (!strcmp(color, "rosso"))
            res = 1;
        else if (!strcmp(color, "verde"))
            res = 2;
        else if (!strcmp(color, "blu"))
            res = 3;
        else if (!strcmp(color, "giallo"))
            res = 5;

        if (res == 0)
            printf("%s, Seleziona un colore valido: ", color);
    }
    free(color);
    return res;
}

void plus(Game* game) {
    bool draw = false;
    draw = true;
    for (int i = 0; i < *(game->SzHands + game->CurrentPlayer); i++)
    {
        if (!strcmp((*(game->Players + game->CurrentPlayer) + i)->front, game->DiscardDeck.front))
            draw = false;
    }

    if (draw)
    {
        // game->Move = '+';
        game->Move = '+';
        return;
    }
}

bool check_draw(Game* game) {
    bool draw = true;
    for (int i = 0; i < *(game->SzHands + game->CurrentPlayer); i++)
    {
        struct card temp = *(*(game->Players + game->CurrentPlayer) + i);
        if (!strcmp(temp.front, game->DiscardDeck.front) || temp.color == game->DiscardDeck.color || temp.color == n)
            draw = false;
    }
    return draw;
}

void help()
{
    system(clear);
    printf("\nPer selezionare la carta che vuoi giocare digitala nel formato *faccia* *colore*\n\n");
    printf("Per consultare le regole del gioco:\n");
    printf("https://www.wikihow.it/Giocare-a-UNO");
}

void clean_stdin()
{
    char c;
    do {
        c = getchar();
    } while (c != '\n' && c != EOF);
}