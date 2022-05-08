#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define SIZE_DECK 108
#define STARTING_HAND_SIZE 7

// caratteri di escape corrispondenti ai vari colori
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define BLUE "\x1b[34m"
#define YELLOW "\x1b[33m"
#define RESET "\x1b[0m"

// per fare cancellare correttamente il terminale alla chiamata di system()
// indipendentemente dal sistema operativo per cui il codice è compilato
#ifdef _WIN32
#define clear "cls"
#else
#define clear "clear"
#endif

enum col
{
    na,
    r,
    g,
    b,
    y,
    n
};

struct card
{
    char front[3];
    enum col color;
};

typedef struct game_state
{
    struct card *Deck;
    int SzDeck;

    struct card **Players;
    int SzPlayers;
    int *SzHands;

    int CurrentPlayer;
    char Move;

    struct card DiscardDeck;

    int Plus;
    bool Rotation;
    bool FirstTurn;
} Game;

// funzioni legate alla logica del gioco
void Start(Game *);
void shuffle(Game *);
int get_players();
struct card *find_empty_space(Game *);
void init_players(Game *);
bool is_over(Game *);
void update(Game *);
void lowercase(char *);

// funzioni legate all'interfaccia
void display(Game *);
const char *displayed_card(struct card *c);
void get_move(Game *);
int choose_color();
void help();
