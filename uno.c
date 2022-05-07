#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define SIZE_DECK 108
#define STARTING_HAND_SIZE 7

// caratteri di escape corrispondenti ai varii colori
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

int get_players();
struct card *shuffle();
struct card *find_empty_space(struct card *deck);
struct card **init_players(struct card *deck, int *szDeck, int szPlayers, int *szHands);
void display(struct card **players, int szPlayers, int current_player, int *szHands, struct card *discarded, bool rotation);
char *color_card(struct card *c);
char get_move(struct card **players, int current_player, int *size_hands, struct card *discarded);
void update(char move, struct card *deck, int *szDeck, struct card **players, int *szPlayers, int *current_player, bool *rotation);

int main()
{
  srand(time(0)); // per valori randomici

  // inizializazione mazzo principale
  struct card *deck = shuffle();
  int size_deck = SIZE_DECK;

  // inizializzazione mani dei giocatori
  int size_players = get_players();
  int *size_hands = (int *)malloc(sizeof(int) * size_players);
  struct card **players = init_players(deck, &size_deck, size_players, size_hands);

  // variabili necessarie per il gioco
  int current_player = 0;
  bool rotation = true; // true = sinistra; false = destra;
  struct card *discarded;
  size_deck--;
  discarded = (deck + size_deck);

  // loop del gioco
  while (*(players + current_player))
  // quando il gioco finisce il mazzo del current_player viene deallocato e portato a NULL
  // quindi la condizione diventa falsa perchè (players + current_player) == NULL == 0
  {
    display(players, size_players, current_player, size_hands, discarded, rotation);
    char move = get_move(players, current_player, size_hands, discarded);
    update(move, deck, &size_deck, players, &size_players, &current_player, &rotation);
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
        pos->front[0] = i + 48; // ascii
        pos->front[1] = '\0';
      }
    }

    // carte stop
    for (int i = 0; i < 2; i++)
    {
      struct card *pos = find_empty_space(deck);
      pos->color = (enum col)c;
      strcpy(pos->front, "S");
    }

    // carte reverse
    for (int i = 0; i < 2; i++)
    {
      struct card *pos = find_empty_space(deck);
      pos->color = (enum col)c;
      strcpy(pos->front, "R");
    }

    // carte +2
    for (int i = 0; i < 2; i++)
    {
      struct card *pos = find_empty_space(deck);
      pos->color = (enum col)c;
      strcpy(pos->front, "+2");
    }
  }

  // carte nere choose
  for (int i = 0; i < 4; i++)
  {
    struct card *pos = find_empty_space(deck);
    pos->color = n;
    strcpy(pos->front, "C");
  }

  // carte nere +4
  for (int i = 0; i < 4; i++)
  {
    struct card *pos = find_empty_space(deck);
    pos->color = n;
    strcpy(pos->front, "+4");
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
struct card **init_players(struct card *deck, int *szDeck, int szPlayers, int *szHands)
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
      strcpy((*(players + i) + j)->front, (deck + (*szDeck))->front);
    }

  for (int i = 0; i < szPlayers; i++)
    *(szHands + i) = STARTING_HAND_SIZE;

  return players;
}

// stampa a video la mano del giocatore corrente e la carta in cima al mazzo discard
void display(struct card **players, int szPlayers, int current_player, int *szHands, struct card *discarded, bool rotation)
{
  system(clear);

  printf("Turno del Giocatore %d\n\n", current_player + 1); // mostra il numero del giocatore corrente

  // mostra il numero di carte nelle mani degli avversari, nell'ordine della rotazione
  for (int i = rotation ? 0 : (szPlayers - 1); (rotation && (i < szPlayers)) || (!rotation && (i >= 0));)
  {
    if (i != current_player)
      printf("il giocatore %d ha %d carte nella sua mano\n", i + 1, *(szHands + i));
    if (rotation)
      i++;
    else
      i--;
  }

  // mostra il verso di rotazione
  printf("\nLa rotazione e' attualmente in senso %s\n\n\n", rotation ? "orario" : "antiorario");

  for (int i = 0; i < (*(szHands + current_player)); i++) // aggiusta il mazzo discard più o meno
    printf("\t");                                         // al centro rispetto alla mano

  char *colored_card = color_card(discarded); // la carta in cima al mazzo discard
  printf("%s\n\n\n\n\n\t", colored_card);

  for (int i = 0; i < (*(szHands + current_player)); i++) // mostra la mano del giocatore
  {
    colored_card = color_card(*(players + current_player) + i);
    printf("%s\t\t", colored_card);
  }

  printf("%s\n\n\n\n", RESET);
  printf("Seleziona la carta che intendi giocare, o seleziona 'aiuto' per consultare le regole: ");
  free(colored_card);
}

// converte una carta in una stringa che ha come prefisso la sequenza di escape
// corrispondente al suo colore
char *color_card(struct card *c)
{
  // imposta il colore
  char color[6];
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
  char *colored_card;
  colored_card = (char *)malloc(sizeof(char) * ((face ? strlen(face) : 2) + strlen(color)));
  strcpy(colored_card, color);
  strcat(colored_card, (face ? face : c->front));

  free(face);
  return colored_card;
}

char get_move(struct card **players, int current_player, int *size_hands, struct card *discarded)
{
  // in caso di nessuna mossa disponibile si pesca
  bool draw = true;
  for (int i = 0; i < *(size_hands + current_player); i++)
  {
    struct card temp = *(*(players + current_player) + i);
    if (!strcmp(temp.front, discarded->front) || temp.color == discarded->color || temp.color == n)
      draw = false;
  }
  if (draw)
    return 'd';

  char *move = (char *)malloc(sizeof(char) * 20);

  while (true)
  {
    struct card chosen;
    chosen.front[0] = '\0'; // inizializzazione variabile
    chosen.color = na;

    scanf("%20[^\n]", move); // per leggere l'intera riga senza fermarsi agli spazi

    char *temp = move; // converte tutti i caratteri maiuscoli e minuscoli per le
    while (*temp)      // future comparazioni
    {
      if (*temp >= 'A' && *temp <= 'Z')
        *temp += 32;
      temp++;
    }

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
    // regolamento
    else if (!strcmp(token, "aiuto"))
      return 'h';

    if (chosen.color == na)
    {
      token = strtok(NULL, "\n"); // prende la seconda parola, corrispondente al colore

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
      if (chosen.front != discarded->front && chosen.color != discarded->color && chosen.color != n)
      {
        printf("La carta non è compatibile con quella in cima al mazzo Discard, selezionane un'altra: ");
      }
      else
      {
        for (int i = 0; i < *(size_hands + current_player); i++)
        {
          struct card temp = *(*(players + current_player) + i);
          if (chosen.front == temp.front && chosen.color == temp.color)
            return i + 48;
        }

        printf("Non hai quella carta, selezionane un'altra: ");
      }
    }
    else
    {
      printf("Seleziona una mossa valida: ");
    }
  }
}

void update(char move, struct card *deck, int *szDeck, struct card **players, int *szPlayers, int *current_player, bool *rotation)
{
  *(players + *current_player) = NULL;
}
