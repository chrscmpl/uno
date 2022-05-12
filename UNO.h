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
    na, // carta non inizializzata
    r,
    g,
    b,
    y,
    w // wild
};

struct card
{
    char front[3];  // simbolo
    enum col color; // colore
};

typedef struct game_state
{
    struct card *Deck; // mazzo principale
    int SzDeck;        // indica le carte rimaste nel mazzo principale

    struct card **Players; // array bidimensionale delle mani dei giocatori
    int SzPlayers;         // indica il numero di giocatori
    int *SzHands;          // array contenente le dimensioni delle mani dei giocatori

    int CurrentPlayer; // indica il giocatore corrente
    char Move;         // mossa del giocatore corrente

    struct card DiscardDeck; // carta in cima al discard deck

    int Plus;       // somma delle carte da pescare date dai +2 o +4 appena giocati
    bool Rotation;  // true == orario    false == antiorario
    bool FirstTurn; // serve per alcune funzioni per capire se è il primo turno
    bool HasDrawn;  // serve ad assicurarsi che se si decide di pescare con una carta compatibile in mano
                    // e si peschi una carta compatibile poi si giochi la carta pescata
    int AIPlay;     // serve all'IA per tenere traccia delle carte che ha giocato
    bool AI;        // true == partita contro l'IA      false == partita tra più giocatori
    bool GameOver;  // permette di terminare il gioco
} Game;

// funzioni legate alla logica del gioco
void Start(Game *);
void shuffle(Game *);
struct card *find_empty_space(Game *);
int get_players();
void init_players(Game *);
void get_move(Game *);
void update(Game *);
struct card chosen_card();
void lowercase(char *);
void next_turn(Game *);
void remove_from_hand(Game *, int);
bool check_draw(Game *);
void draw(Game *, int);
void plus(Game *);
bool forgot_uno();
void first_turn_effects(Game *);
void refill(Game *);
void end_game(Game *);
struct card AI_turn(Game *);
bool is_AI(Game *);

// funzioni legate all'interfaccia
void display(Game *);
void transition(Game *);
const char *displayed_card(struct card *);
void read_words(char *);
int choose_color();
void show_drawn(Game *, int);
void help();
void show_winner(int);
bool play_again();
void display_message(char *);
void clean_stdin();