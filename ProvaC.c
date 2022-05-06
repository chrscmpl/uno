#include <stdio.h>

#define DIM_DECK 108

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
struct card *find_empty_space(struct card *);

int main()
{
  struct card *deck;
  struct card **players;
  int dim_players = get_players();

  deck = shuffle();

  return 0;
}

int get_players()
{
  int p = 0;
  printf("inserire il numero di giocatori: ");
  while (p < 1 || p > 4)
  {
    scanf("%d", &p);
    if (p < 1 || p > 4)
      printf("numero di giocatori non valido, inserire un numero di giocatori compreso tra 1 e 4: ");
  }
  system(clear);
  return p;
}

// inizializza il mazzo principale
struct card *shuffle()
{
  struct card *deck = (struct card *)malloc(sizeof(struct card) * DIM_DECK);

  for (int i = 0; i < DIM_DECK; i++) // inizializza il mazzo
  {
    (deck + i)->color = na;
  }

  for (int c = 1; c < 5; c++) // per ogni colore
  {
    for (int i = 0; i < 10; i++) // carte numero
    {
      for (int j = 0; j == 0 || (j == 1 && i != 0); j++) // c'è solo uno zero per colore
      {
        struct card *pos = find_empty_space(deck);
        pos->color = c;
        pos->face[0] = i + 48; // ascii
        pos->face[1] = '\0';
      }
    }

    for (int i = 0; i < 2; i++) // carte stop
    {
      struct card *pos = find_empty_space(deck);
      pos->color = c;
      pos->face[0] = 'S';
      pos->face[1] = '\0';
    }

    for (int i = 0; i < 2; i++) // carte reverse
    {
      struct card *pos = find_empty_space(deck);
      pos->color = c;
      pos->face[0] = 'R';
      pos->face[1] = '\0';
    }

    for (int i = 0; i < 2; i++) // carte +2
    {
      struct card *pos = find_empty_space(deck);
      pos->color = c;
      pos->face[0] = '+';
      pos->face[1] = '2';
      pos->face[2] = '\0';
    }
  }

  for (int i = 0; i < 4; i++) // carte nere choose
  {
    struct card *pos = find_empty_space(deck);
    pos->color = n;
    pos->face[0] = 'C';
    pos->face[1] = '\0';
  }

  for (int i = 0; i < 4; i++) // carte nere +4
  {
    struct card *pos = find_empty_space(deck);
    pos->color = n;
    pos->face[0] = '+';
    pos->face[1] = '4';
    pos->face[2] = '\0';
  }

  return deck;
}

struct card *find_empty_space(struct card *deck)
{
  struct card *space;
  do
  {
    space = (deck + (rand() % DIM_DECK));
  } while (space->color != na);
  return space;
}