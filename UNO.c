#include "UNO.h"

void Start(Game *game)
{
    game->GameOver = false;

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

// richiede dalla tastiera l'inserimento del numero di giocatori
int get_players()
{
    system(clear);
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

//  imposta il valore di game->move
//  - a 'd' se bisogna pescare
//  - ad 'u' se non si è detto uno quando si doveva
//  - ad 'h' se è stato chiesto di consultare le regole
//  - ad un numero corrispondente alla posizione della carta da giocare
//    nella mano del giocatore se si è selezionata una carta valida
//  Inoltre chiama plus() per gestire i +2 e +4
void get_move(Game *game)
{
    if (game->FirstTurn) {
        first_turn_effects(game);
        return;
    }

    // +2 e +4
    if (game->Plus)
    {
        plus(game);
        return;
    }

    // in caso di nessuna mossa disponibile si pesca
    if (check_draw(game))
    {
        game->Move = 'd';
        return;
    }

    // ricorda di dire Uno!
    if (*(game->SzHands + game->CurrentPlayer) == 1 && forgot_uno())
    {
        game->Move = 'u';
        return;
    }

    while (true)
    {

        struct card chosen = chosen_card();

        // se si è digitato 'aiuto'
        if (chosen.front[0] == 'h')
        {
            game->Move = 'h';
            return;
        }

        // se la carta è valida
        if (chosen.front && chosen.color != na)
        {

            // confronto con quella in cima al mazzo discard
            if (strcmp(chosen.front, game->DiscardDeck.front) && chosen.color != game->DiscardDeck.color && chosen.color != n && game->DiscardDeck.color != n)
            {
                display_message("La carta non e' compatibile con quella in cima al mazzo Discard, selezionane un'altra");
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

                display_message("Non hai quella carta, selezionane un'altra");
            }
        }
        else
        {
            display_message("Seleziona una mossa valida");
        }
    }
}

void update(Game *game)
{
    struct card *player = *(game->Players + game->CurrentPlayer); // per leggibilità
    int *szHand = (game->SzHands + game->CurrentPlayer);
    int played = game->Move - 48;
    bool stop = false;

    // help e pescate
    switch (game->Move)
    {
    case ' ':
        return;
    case 'h':
        help();
        return;
    case 'd':
        draw(game, 1);
        show_drawn(game, 1);
        struct card drawn = *(player + *szHand - 1);
        if(strcmp(drawn.front, game->DiscardDeck.front) && (drawn.color != game->DiscardDeck.color) && (drawn.color != n))
            next_turn(game);
        return;
    case 'u':
        draw(game, 2);
        display_message("Hai dimenticato di dire UNO!");
        show_drawn(game, 2);
        next_turn(game);
        return;
    case '+':
        draw(game, game->Plus);
        show_drawn(game, game->Plus);
        game->Plus = 0;
        return;
    }

    // aggiorna il mazzo discard
    game->DiscardDeck = *(player + played);

    // effetti delle carte
    switch ((player + played)->front[0])
    {
    case 'S':
        stop = true;
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

    remove_from_hand(game, played);

    // controlla vittoria
    if (!(*szHand)) {
        game->GameOver = true;
        return;
    }

    next_turn(game);

    if (stop)
        next_turn(game);

    fflush(stdin);
    game->Move = ' ';
}

// può restituire
//  - una carta valida
//  -una carta non valida
//  -una carta con faccia h per help
struct card chosen_card()
{
    char move[20];

    struct card chosen;
    chosen.front[0] = '\0'; // inizializzazione variabile
    chosen.color = na;

    read_words(move);

    // regolamento
    if (!strcmp(move, "aiuto"))
    {
        chosen.front[0] = 'h';
        return chosen;
    }

    short spazi = 0;
    char *temp = move; // converte tutti i caratteri maiuscoli e minuscoli per le
    while (*temp)      // future comparazioni e controlla il numero di spazi
    {
        if (*temp == ' ')
            spazi++;
        temp++;
    }

    if (spazi != 1 && strcmp(move, "choose") && strcmp(move, "+4")) // poichè per le nere non serve specificare il colore non ci sono spazi
        return chosen;

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

    return chosen;
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

// passa il turno
void next_turn(Game *game)
{
    if (game->Rotation)
    {
        game->CurrentPlayer++;
        if (game->CurrentPlayer >= game->SzPlayers)
            game->CurrentPlayer -= game->SzPlayers;
    }
    else
    {
        game->CurrentPlayer--;
        if (game->CurrentPlayer < 0)
            game->CurrentPlayer += game->SzPlayers;
    }
}

// aggiorna la mano
void remove_from_hand(Game *game, int played)
{
    for (int i = played; i < *(game->SzHands + game->CurrentPlayer) - 1; i++)
        *(*(game->Players + game->CurrentPlayer) + i) = *(*(game->Players + game->CurrentPlayer) + i + 1);

    (*(game->SzHands + game->CurrentPlayer))--;
}

bool check_draw(Game *game)
{
    if (game->DiscardDeck.color == n) // solo nel caso del primo turno
        return false;

    bool draw = true;
    for (int i = 0; i < *(game->SzHands + game->CurrentPlayer); i++)
    {
        struct card temp = *(*(game->Players + game->CurrentPlayer) + i);
        if (!strcmp(temp.front, game->DiscardDeck.front) || temp.color == game->DiscardDeck.color || temp.color == n)
            draw = false;
    }
    return draw;
}

// pesca e aggiunge alla mano del giocatore n carte
void draw(Game *game, int n)
{
    while (n > 0)
    {
        if (!game->SzDeck) //se il mazzo termina le carte
        {
            display_message("Che partita! Il mazzo principale e' appena stato riempito di nuovo!");
            refill(game);
        }

        // copia la carta in cima alla mano del giocatore
        game->SzDeck--;

        ((*(game->Players + game->CurrentPlayer) + *(game->SzHands + game->CurrentPlayer)))->color = (game->Deck + game->SzDeck)->color;
        strcpy((*(game->Players + game->CurrentPlayer) + *(game->SzHands + game->CurrentPlayer))->front, (game->Deck + game->SzDeck)->front);

        // aumenta la dimensione della mano
        (*(game->SzHands + game->CurrentPlayer))++;

        n--;
    }

}


void plus(Game *game)
{
    bool draw = false;
    draw = true;
    for (int i = 0; i < *(game->SzHands + game->CurrentPlayer); i++)
    {
        if (!strcmp((*(game->Players + game->CurrentPlayer) + i)->front, game->DiscardDeck.front))
            draw = false;
    }

    if (draw)
    {
        game->Move = '+';
        return;
    }
    
    //Nel caso il giocatore abbia in mano un +2 / +4
    while (true)
    {

        struct card chosen = chosen_card();

        // se si è digitato 'aiuto'
        if (chosen.front[0] == 'h')
        {
            game->Move = 'h';
            return;
        }

        // se la carta è valida
        if (chosen.front && chosen.color != na)
        {
            bool in_hand = false;
            // confronto con quelle nella mano del giocatore
            for (int i = 0; i < *(game->SzHands + game->CurrentPlayer); i++)
            {
                struct card temp = *(*(game->Players + game->CurrentPlayer) + i);
                if (!strcmp(temp.front, chosen.front) && chosen.color == temp.color)
                {
                    game->FirstTurn = false;
                    game->Move = i + 48;
                    in_hand = true;
                }
            }
            
            if(!in_hand)
                display_message("Non hai quella carta, selezionane un'altra");
            else {
                if (!strcmp(chosen.front, game->DiscardDeck.front))
                    return;

                char ch[30] = "Gioca il tuo ";
                strcat(ch, (strcmp(game->DiscardDeck.front, "+2") ? "+4" : "+2"));
                strcat(ch, "!");

                display_message(ch);
            }

        }
        else
        {
            display_message("Seleziona una mossa valida");
        }
    }
}

bool forgot_uno()
{
    char words[20];
    read_words(words);

    if (!(strcmp(words, "uno") && strcmp(words, "uno!")))
        printf("\n\n%sBravo%s, %sla %svittoria %se' %stua%s!%s\n\n", RED, GREEN, BLUE, YELLOW, RED, GREEN, BLUE, RESET);

    return (strcmp(words, "uno") && strcmp(words, "uno!"));
}

void first_turn_effects(Game* game)
{
    game->FirstTurn = false;

    switch (game->DiscardDeck.front[0]) {
    case 'S':

        clean_stdin();

        if (game->Rotation) // il senso dovrebbe essere sempre in senso orario ma magari potrei
        {                   // cambiarlo quindi meglio scrivere le cose per bene
            game->CurrentPlayer++;
            if (game->CurrentPlayer >= game->SzPlayers)
                game->CurrentPlayer -= game->SzPlayers;
        }
        else
        {
            game->CurrentPlayer--;
            if (game->CurrentPlayer < 0)
                game->CurrentPlayer += game->SzPlayers;
        }
        break;
    case 'R':
        game->Rotation = !game->Rotation;
        break;
    case'+':
        game->Plus += game->DiscardDeck.front[1] - 48;
    }
    game->Move = ' ';
}

//nel caso raro in cui il mazzo principale finisca le carte
void refill(Game* game)
{
    free(game->Deck);
    game->Deck = (struct card*)malloc(sizeof(struct card) * SIZE_DECK);
    game->SzDeck = SIZE_DECK;
    shuffle(game);
}

//termina il gioco deallocando la memoria occupata da game e le sue variabili
void end_game(Game* game) {

    show_winner(game->CurrentPlayer + 1);

    free(game->Deck);

    for (int i = 0; i < game->SzPlayers; i++)
        free(*(game->Players + i));

    free(game->Players);

    free(game->SzHands);

    free(game);
}

/******************************************************************************
*************************FUNZIONI DELL'INTERFACCIA*****************************
******************************************************************************/

// stampa a video la mano del giocatore corrente e la carta in cima al mazzo discard
// oltre a varie informazioni
void display(Game* game)
{
    transition(game);
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

    for (int i = 0; (i < (*(game->SzHands + game->CurrentPlayer))) && (i < STARTING_HAND_SIZE); i++) // aggiusta il mazzo discard più o meno
        printf("\t");                                                                                // al centro rispetto alla mano

    // la carta in cima al mazzo discard
    printf("%s\n\n\n\n\n\t", displayed_card(&game->DiscardDeck));

    for (int i = 0; i < (*(game->SzHands + game->CurrentPlayer)); i++) // mostra la mano del giocatore
        printf("%s\t\t", displayed_card(*(game->Players + game->CurrentPlayer) + i));

    printf("%s\n\n\n\n", RESET);
}

//crea una transizione tra i turni dei vari giocatori così non ci si vede la mano a vicenda
void transition(Game* game) {

    //ho bisogno di questa variabile per non mostrare la schermata dopo una pescata
    //o dopo uno stop in una partita tra due giocatori
    static int player;
    if (game->FirstTurn)
        player = -1;

    if (game->CurrentPlayer == player)
        return;

    player = game->CurrentPlayer;

    char color[6] = RESET;
    switch (game->CurrentPlayer) {
    case 0:
        strcpy(color, RED);
        break;
    case 1:
        strcpy(color, GREEN);
        break;
    case 2:
        strcpy(color, BLUE);
        break;
    case 3:
        strcpy(color, YELLOW);
    }

    system(clear);

    printf("\n\n\n\t\t\t\t\t%sTurno del giocatore %d%s", color, game->CurrentPlayer + 1, RESET);
    clean_stdin();
}

// converte una carta in una stringa che ha come prefisso la sequenza di escape
// corrispondente al suo colore
const char* displayed_card(struct card* c)
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
    }

    // per le carte speciali
    char* face = NULL;
    switch (c->front[0])
    {
    case 'S':
        face = (char*)malloc(sizeof(char) * 5);
        strcpy(face, "Stop");
        break;
    case 'R':
        face = (char*)malloc(sizeof(char) * 8);
        strcpy(face, "Reverse");
        break;
    case 'C':
        face = (char*)malloc(sizeof(char) * 7);
        strcpy(face, "Choose");
    }

    // compone la stringa
    static char colored_card[13];
    strcpy(colored_card, color);
    strcat(colored_card, (face ? face : c->front));

    free(face);
    return colored_card;
}

void read_words(char *str)
{
    char words[20];

    printf("Seleziona la carta che intendi giocare,\noppure digita 'aiuto' per consultare le regole: ");

    scanf("%20[^\n]", words); // per leggere l'intera riga senza fermarsi agli spazi
    clean_stdin();

    lowercase(words);

    strcpy(str, words);
}

int choose_color()
{
    printf("\nScegli il colore della carta giocata: ");
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
            res = 4;

        if (res == 0)
            printf("\nSeleziona un colore valido: ");
    }
    free(color);
    return res;
}

void show_drawn(Game *game, int n)
{
    printf("Hai pescato %s:\n", (n > 1 ? "le seguenti carte" : "la seguente carta"));
    struct card *p = *(game->Players + game->CurrentPlayer) + *(game->SzHands + game->CurrentPlayer);

    p -= n;
    for (int i = 0; i < n; i++)
    {
        printf("%s\t\t", displayed_card(p));
        p++;
    }

    printf("%s\n", RESET);
    clean_stdin(); // system("pause")
}

void help()
{
    system(clear);

    // le regole vengono lette dal file rules.txt
    FILE* rules;

    rules = fopen("rules.txt", "r");
    char ch[200];

    while (!feof(rules)) {
        fgets(ch, 200, rules);
        puts(ch);
    }

    fclose(rules);
    clean_stdin();
}

void show_winner(int winner) {
    system(clear);

    printf("\n\n\n\t\t\t\t\t%sVince %sil %sgiocatore%s %d%s!%s", RED, GREEN, BLUE, YELLOW, winner, RED, RESET);

    clean_stdin();
}

bool play_again() {
    bool answered = false;
    bool again = false;
    while (!answered)
    {
        system(clear);

        printf("\n\n\n\t\t\t\t\t%sVuoi %sgiocare %sancora%s?%s\n\n\t\t\t\t\t", RED, GREEN, BLUE, YELLOW, RESET);

        char answer[20];
        scanf("%s", answer);
        clean_stdin();
        lowercase(answer);

        if (!strcmp(answer, "si")) {
            again = true;
            answered = true;
        }
        else if (!strcmp(answer, "no"))
            answered = true;
    }

    return again;
}
//l'unico scopo di questa funzione e' separare il piú possibile le funzioni
//legate alla logica del gioco con il modo in cui le informazioni sono
//rappresentate a schermo
void display_message(char* str)
{
    char ch[200] = "\n";
    strcat(ch, str);
    strcat(ch, "\n\n");
    printf(ch);
}


//scanf per qualche motivo crea loop infiniti perchè il buffer di input non viene pulito completamente.
//Ho provato fflush(stdin) ma funzionava solo su un compilatore e su un altro no.
//questa funzione sostituisce fflush(stdin) e funziona su ogni compilatore,
//ma se usata quando il buffer è già vuoto, attende che l'utente prema invio prima di proseguire,
//motivo per il quale il suo secondo lavoro è quello di sostituire system("pause"),
//poichè pause è un comando solo di Windows e non funziona su Linux e MacOs
void clean_stdin()
{
    char c;
    do
    {
        c = getchar();
    } while (c != '\n' && c != EOF);
}
