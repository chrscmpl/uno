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
void lowercase(char *);
char get_move(struct card **players, int current_player, int *size_hands, struct card *discarded);
void update(char move, struct card *deck, int *szDeck, struct card **players, int *szPlayers, int *size_hands, int *current_player, bool *rotation, struct card *discarded);
int choose_color();
void help();

bool primo_turno = true; // le variabili globali non sono buone ma mi serviva per i +2 e +4

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
    update(move, deck, &size_deck, players, &size_players, size_hands, &current_player, &rotation, discarded);
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

char get_move(struct card **players, int current_player, int *size_hands, struct card *discarded)
{
  // +2 e +4
  bool draw = false;
  if ((!strcmp(discarded->front, "+2") || !strcmp(discarded->front, "+4")) && !primo_turno)
  {
    draw = true;
    for (int i = 0; i < *(size_hands + current_player); i++)
    {
      if (!strcmp((*(players + current_player) + i)->front, discarded->front))
        draw = false;
    }
  }
  if (draw)
    return '+';

  // in caso di nessuna mossa disponibile si pesca
  draw = true;
  for (int i = 0; i < *(size_hands + current_player); i++)
  {
    struct card temp = *(*(players + current_player) + i);
    if (!strcmp(temp.front, discarded->front) || temp.color == discarded->color || temp.color == n)
      draw = false;
  }
  if (draw)
    return 'd';

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
      return 'h';

    // ricorda di dire Uno!
    if (strcmp(move, "uno") && *(size_hands + current_player) == 1)
      return 'u';

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
        if (((!strcmp(discarded->front, "+2") || !strcmp(discarded->front, "+4")) && strcmp(chosen.front, discarded->front)) && !primo_turno)
        {
          printf("Gioca il tuo %s", (strcmp(chosen.front, "+2") ? "+4" : "+2"));
        }
        else
        {
          // confronto con quella in cima al mazzo discard
          if (strcmp(chosen.front, discarded->front) && chosen.color != discarded->color && chosen.color != n && discarded->color != n)
          {
            printf("La carta non e' compatibile con quella in cima al mazzo Discard, selezionane un'altra: ");
          }
          else
          {
            // confronto con quelle nella mano del giocatore
            for (int i = 0; i < *(size_hands + current_player); i++)
            {
              struct card temp = *(*(players + current_player) + i);
              if (!strcmp(temp.front, chosen.front) && chosen.color == temp.color)
              {
                primo_turno = false;
                return i + 48;
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

void update(char move, struct card *deck, int *szDeck, struct card **players, int *szPlayers, int *size_hands, int *current_player, bool *rotation, struct card *discarded)
{
  struct card *player = *(players + *current_player); // leggibilità
  int *szHand = (size_hands + *current_player);
  int played = move - 48;
  short stop = 0;
  // help e pescate
  switch (move)
  {
  case 'h':
    help();
    break;
    // case 'p':
    //   break;
  }

  // aggiorna il mazzo discard
  *discarded = *(player + played);

  // effetti delle carte
  switch ((player + played)->front[0])
  {
  case 'S':
    stop = 1;
    break;
  case 'R':
    *rotation = !(*rotation);
    break;
  case 'C':
    discarded->color = (enum col)choose_color();
  }

  // aggiorna la mano
  for (int i = played; i < *szHand - 1; i++)
    *(player + i) = *(player + i + 1);
  (*szHand)--;

  // passa il turno
  if (*rotation)
    *current_player = (*current_player < *szPlayers - 1 ? (*current_player + 1 + stop) : (0 + stop));
  else
    *current_player = (*current_player > 0 ? (*current_player - 1 - stop) : (*szPlayers - 1 - stop));
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
