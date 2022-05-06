#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define SIZE_DECK 108
#define STARTING_HAND_SIZE 7

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
  char face[3];
  enum col color;
};

int get_players();
struct card *shuffle();
struct card *find_empty_space(struct card *deck);
struct card **init_players(struct card *deck, int *szDeck, int szPlayers);
void turn(struct card *deck, int *szDeck, struct card **players, int *szPlayers, int *current_player, bool *rotation);

int main()
{
  struct card *deck;
  struct card **players;

  deck = shuffle();
  int size_deck = SIZE_DECK;
  /*
  for (int i = 0; i < size_deck; i++)
  {
    printf("f= %s\tc= %d\n", (deck + i)->face, (deck + i)->color);
  }
  */
  int size_players = get_players();
  players = init_players(deck, &size_deck, size_players);
  /*
  for (int i = 0; i < size_players; i++)
  {
    printf("mano del giocatore %d:\n", i);
    for (int j = 0; j < STARTING_HAND_SIZE; j++)
      printf("\t%s\t%d\n", (*(players + i) + j)->face, (*(players + i) + j)->color);
  }
  */
  int current_player = 0;
  bool rotation = true; // true = sinistra; false = destra;

  while (players + current_player)
  // quando il gioco finisce il mazzo del current_player viene deallocato e portato a NULL
  // quindi la condizione diventa falsa perchè (players + current_player) == NULL == 0
  {
    turn(deck, &size_deck, players, &size_players, &current_player, &rotation);
  }

  return 0;
}

// richiede dalla tastiera l'inserimento del numero di giocatori
int get_players()
{
  int p = 0;
  printf("inserire il numero di giocatori: ");
  while (p < 2 || p > 4)
  {
    scanf("%d", &p);
    if (p < 2 || p > 4)
      printf("numero di giocatori non valido, inserire un numero di giocatori compreso tra 2 e 4: ");
  }
  system(clear);
  return p;
}

// inizializza il mazzo principale
struct card *shuffle()
{
  struct card *deck = (struct card *)malloc(sizeof(struct card) * SIZE_DECK);

  for (int i = 0; i < SIZE_DECK; i++) // inizializza il mazzo
  {
    (deck + i)->color = na;
  }

  for (int c = 1; c < 5; c++) // per ogni colore
  {
    // carte numero
    for (int i = 0; i < 10; i++)
    {
      for (int j = 0; j == 0 || (j == 1 && i != 0); j++) // c'è solo uno zero per colore
      {
        struct card *pos = find_empty_space(deck);
        pos->color = (enum col)c;
        pos->face[0] = i + 48; // ascii
        pos->face[1] = '\0';
      }
    }

    // carte stop
    for (int i = 0; i < 2; i++)
    {
      struct card *pos = find_empty_space(deck);
      pos->color = (enum col)c;
      strcpy(pos->face, "S");
    }

    // carte reverse
    for (int i = 0; i < 2; i++)
    {
      struct card *pos = find_empty_space(deck);
      pos->color = (enum col)c;
      strcpy(pos->face, "R");
    }

    // carte +2
    for (int i = 0; i < 2; i++)
    {
      struct card *pos = find_empty_space(deck);
      pos->color = (enum col)c;
      strcpy(pos->face, "+2");
    }
  }

  // carte nere choose
  for (int i = 0; i < 4; i++)
  {
    struct card *pos = find_empty_space(deck);
    pos->color = n;
    strcpy(pos->face, "C");
  }

  // carte nere +4
  for (int i = 0; i < 4; i++)
  {
    struct card *pos = find_empty_space(deck);
    pos->color = n;
    strcpy(pos->face, "+4");
  }

  return deck;
}

// cerca spazi vuoti nel mazzo
struct card *find_empty_space(struct card *deck)
{
  struct card *space;
  do
  {
    space = (deck + (rand() % SIZE_DECK));
  } while (space->color != na);
  return space;
}

// inizializza i mazzi dei giocatori
struct card **init_players(struct card *deck, int *szDeck, int szPlayers)
{
  // inizializza l'array di mazzi
  struct card **players = (struct card **)malloc(sizeof(struct card *) * szPlayers);

  // inizializza i singoli mazzi
  for (int i = 0; i < szPlayers; i++)
    *(players + i) = (struct card *)malloc(sizeof(struct card) * (*szDeck)); // alloca lo spazio necessario

  for (int i = 0; i < szPlayers; i++) // pesca carte dal mazzo
    for (int j = 0; j < STARTING_HAND_SIZE; j++)
    {
      (*szDeck)--;
      ((*(players + i)) + j)->color = (deck + (*szDeck))->color;
      strcpy((*(players + i) + j)->face, (deck + (*szDeck))->face);
    }

  return players;
}

void turn(struct card *deck, int *szDeck, struct card **players, int *szPlayers, int *current_player, bool *rotation)
{
  printf("\nhello\n");
}
