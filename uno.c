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

typedef struct game_state
{
  struct card *Deck;
  int SzDeck;

  struct card **Players;
  int SzPlayers;
  int *SzHands;

  int CurrentPlayer;
  char move;

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
char *displayed_card(struct card *c);
void get_move(Game *);
int choose_color();
void help();

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
  // quando il gioco finisce il mazzo del current_player viene deallocato e portato a NULL
  // quindi la condizione diventa falsa perchè (players + current_player) == NULL == 0
  return !(*(game->Players + game->CurrentPlayer));
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
    printf("\t");                                                    // al centro rispetto alla mano

  char *colored_card = displayed_card(&game->DiscardDeck); // la carta in cima al mazzo discard
  printf("%s\n\n\n\n\n\t", colored_card);

  for (int i = 0; i < (*(game->SzHands + game->CurrentPlayer)); i++) // mostra la mano del giocatore
  {
    colored_card = displayed_card(*(game->Players + game->CurrentPlayer) + i);
    printf("%s\t\t", colored_card);
  }

  printf("%s\n\n\n\n", RESET);
  free(colored_card);
}

// converte una carta in una stringa che ha come prefisso la sequenza di escape
// corrispondente al suo colore
char *displayed_card(struct card *c)
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
  bool draw = false;
  if ((!strcmp(game->DiscardDeck.front, "+2") || !strcmp(game->DiscardDeck.front, "+4")) && !game->FirstTurn)
  {
    draw = true;
    for (int i = 0; i < *(game->SzHands + game->CurrentPlayer); i++)
    {
      if (!strcmp((*(game->Players + game->CurrentPlayer) + i)->front, game->DiscardDeck.front))
        draw = false;
    }
  }
  if (draw)
  {
    game->move = '+';
    return;
  }

  // in caso di nessuna mossa disponibile si pesca
  draw = true;
  for (int i = 0; i < *(game->SzHands + game->CurrentPlayer); i++)
  {
    struct card temp = *(*(game->Players + game->CurrentPlayer) + i);
    if (!strcmp(temp.front, game->DiscardDeck.front) || temp.color == game->DiscardDeck.color || temp.color == n)
      draw = false;
  }
  if (draw)
  {
    game->move = 'd';
    return;
  }

  printf("Seleziona la carta che intendi giocare,\no seleziona 'aiuto' per consultare le regole: ");

  char *move = (char *)malloc(sizeof(char) * 20);

  while (true)
  {
    struct card chosen;
    chosen.front[0] = '\0'; // inizializzazione variabile
    chosen.color = na;

    fflush(stdin); // mi ha salvato da un loop infinito

    scanf("%20[^\n]", move); // per leggere l'intera riga senza fermarsi agli spazi
    fflush(stdin);

    lowercase(move);

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
      game->move = 'h';
      return;
    }

    // ricorda di dire Uno!
    if (strcmp(move, "uno") && *(game->SzHands + game->CurrentPlayer) == 1)
    {
      game->move = 'u';
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

      free(move);
      // controlla la validità della mossa
      if (chosen.front && chosen.color != na)
      {
        // si assicura che nel caso in cui ci sia un +2 o +4 e il giocatore possa rispondere lo faccia
        if (((!strcmp(game->DiscardDeck.front, "+2") || !strcmp(game->DiscardDeck.front, "+4")) && strcmp(chosen.front, game->DiscardDeck.front)) && !game->FirstTurn)
        {
          printf("Gioca il tuo %s", (strcmp(chosen.front, "+2") ? "+4" : "+2"));
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
                game->move = i + 48;
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

void update(Game *game)
{
  struct card *player = *(game->Players + game->CurrentPlayer); // leggibilità
  int *szHand = (game->SzHands + game->CurrentPlayer);
  int played = game->move - 48;
  short stop = 0;
  // help e pescate
  switch (game->move)
  {
  case 'h':
    help();
    break;
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
    game->CurrentPlayer = (game->CurrentPlayer < game->SzPlayers - 1 ? (game->CurrentPlayer + 1 + stop) : (0 + stop));
  else
    game->CurrentPlayer = (game->CurrentPlayer > 0 ? (game->CurrentPlayer - 1 - stop) : (game->SzPlayers - 1 - stop));
  fflush(stdin);
}

int choose_color()
{
  printf("Scegli il colore della carta giocata: ");
  char *color = (char *)malloc(sizeof(char) * 10);

  int res = 0;
  while (res == 0)
  {
    scanf("%s", color);
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
      printf("Seleziona un colore valido: ");
  }
  free(color);
  return res;
}

void help()
{
  system(clear);
  printf("\nPer selezionare la carta che vuoi giocare digitala nel formato *faccia* *colore*\n\n");
  printf("Per consultare le regole del gioco:\n");
  printf("https://www.wikihow.it/Giocare-a-UNO");
}
