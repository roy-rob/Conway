//Bibliothek zum Rendern des Programms und zur Interaktion der Visualisierung mit dem Betriebssystem
#include <SDL3/SDL.h>

//Ben�tigte C-Bibliotheken
#include <bits/time.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

//OMP-Bibliothek zum Parallelisieren
#include <omp.h>

//Infos zum Fenster
#define TITLE "Conway's Game of Life"
#define WIDTH 1000
#define HEIGHT 1000
#define GRID_HEIGHT 4000
#define GRID_WIDTH 4000

//Nummer der Threads
#define NUM_THREADS 12

//SDL-FLAGS
Uint32 INIT_FLAGS = SDL_INIT_VIDEO;
Uint32 WINDOW_FLAGS = SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALWAYS_ON_TOP;

int currentheight = WIDTH;
int currentwidth = HEIGHT;

//Rate der "lebenden" Zellen am Anfang des Spiels
int weissrate = 5;

// Infos zum gerenderten Ausschnitt des gesamten Spielfeldes
int CAMERA_GRID_X = 250;
int CAMERA_GRID_Y = 250;
int CAMERA_COORD_X = GRID_WIDTH / 2;
int CAMERA_COORD_Y = GRID_HEIGHT / 2;


// Prim�re Datenstruktur
typedef struct {
  SDL_FRect rect;
  bool weiss;
  int lebendeNachbarn;
} punkt;


//Funktion zum Erzeugen des Spielfeldes
void gridErzeugen(punkt *grid) {
  
  for (int i = 0; i < GRID_HEIGHT; i++) {
    for (int j = 0; j < GRID_WIDTH; j++) {

      punkt *elem = &grid[i * GRID_WIDTH + j];
      //Zuf�llige Zuweisung von lebenden Zellen. Weissrate = 5 bedeutet dass ca. 20% aller Zellen zu Beginn "leben".
      elem->weiss = (rand() % weissrate == 0);
      
    }
  }
}

//Visualisierungsfunktion. Rendert das Spiel.
void Draw(SDL_Renderer *renderer, punkt *grid) {

  //Render-State auf Schwarz setzen
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  //Gesamtes Fenster schwarz zeichnen
  SDL_RenderClear(renderer);

  //Render-State auf Wei� setzen. Legt Farbe der lebenden Zellen fest.  
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

  //Hilfsvariablen um den gerenderten Ausschnitt des gr��eren Grids zu bestimmen.
  int anfang_x = CAMERA_COORD_X - (CAMERA_GRID_X / 2);
  int ende_x = CAMERA_COORD_X + (CAMERA_GRID_X / 2);
  int anfang_y = CAMERA_COORD_Y - (CAMERA_GRID_Y / 2);
  int ende_y = CAMERA_COORD_Y + (CAMERA_GRID_Y / 2);

  //Gr��e der gerenderten Zellen festlegen
  int punktbreite = currentwidth / CAMERA_GRID_X;
  int punkthohe = currentheight / CAMERA_GRID_Y;

  //Bereich des darzustellenden Ausschnittes aus dem gesamten Grid iterieren. 
  for (int i = anfang_y; i < ende_y; i++) {
    int aktuelly = (i - anfang_y) * punkthohe;
    for (int j = anfang_x; j < ende_x; j++) {
      punkt *elem = &grid[i * GRID_WIDTH + j];
 
      int aktuellx = (j - anfang_x) * punktbreite;

      // Nur die Position und Gr��e von lebenden Zellen werden berechnet. 
      if (elem->weiss) {
	SDL_FRect rect;
	rect.x = (float)aktuellx;
	rect.w = (float)punktbreite;
	rect.y = (float)aktuelly;
        rect.h = (float)punkthohe;
	//Lebende Zellen einf�rben.
        SDL_RenderFillRect(renderer, &rect);        
      }
    }  
  }
  //Berechnete Visualisierung des Spiels pr�sentieren. (Information mit lebenden Zellen an Bildschirm schicken)
  SDL_RenderPresent(renderer);
}

//Lebende Nachbarn jeder Zelle z�hlen
void lebendeNachbarnZaehlen(punkt *grid) {
  //OMP-Parallelisierung starten
  #pragma omp parallel for schedule(static)
  for (int i = 0; i < GRID_HEIGHT; i++) {
    for (int j = 0; j < GRID_WIDTH; j++) {
      //Zelle ausw�hlen
      punkt *elem = &grid[i * GRID_WIDTH + j];

      //Z�hler f�r die lebenden Nachbarn
      int lebendeNachbarn = 0;

      //Umliegende Zellen pr�fen
      for (int di = -1; di <= 1; di++) {
        for (int dj = -1; dj <= 1; dj++) {
	  if (di == 0 && dj == 0) continue;
	  int ni = i + di;
          int nj = j + dj;

          //Wenn Nachbarzelle au�erhalb des Spielfeldes liegt, �berspringe
          if (ni < 0 || ni >= GRID_HEIGHT || nj < 0 || nj >= GRID_WIDTH)
            continue;
	  //Wenn Nachbarzelle lebt, erh�he den Z�hler
          if (grid[ni * GRID_WIDTH + nj].weiss)
            ++lebendeNachbarn;          
        }
      }
      //Schreibe Informationen in Struct
      elem->lebendeNachbarn = lebendeNachbarn;
    }
  }
}

//Status "lebend/nichtlebend" festlegen
void Spielen(punkt *grid) {
  // OMP-Parallelisierung starten  
  #pragma omp parallel for schedule(static)
  for (int i = 0; i < GRID_HEIGHT; i++) {
    for (int j = 0; j < GRID_WIDTH; j++) {
      //Aktuelle Zelle ausw�hlen
      punkt *elem = &grid[i * GRID_WIDTH + j];

      //Boolean "weiss" gleich dem Resultat der dahinterliegenden Bedingung festlegen. Regeln von Conway's Game of Life
      if (elem->weiss) {
	//Hat eine lebende Zelle zwei oder drei lebende Nachbarn, lebt sie im n�chsten Zyklus
        elem->weiss = (elem->lebendeNachbarn == 2 || elem->lebendeNachbarn == 3);
      } else {
	//Hat eine tote Zelle drei lebende Nachbarn, lebt sie im n�chsten Zyklus
        elem->weiss = (elem->lebendeNachbarn == 3);
      }
      
    }                 
  }
}


int main() {
  //Rand festlegen (Beim Start des Programms sind immer die gleichen Zellen lebend)
  srand(1);

  //Zahl der OMP-Threads festlegen
  omp_set_num_threads(NUM_THREADS);  

  //SDL initialisieren
  SDL_Init(INIT_FLAGS);
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Event event;

  //Fenster erzeugen und dem Renderer zur Verf�gung stellen
  SDL_CreateWindowAndRenderer(TITLE, WIDTH, HEIGHT, WINDOW_FLAGS, &window, &renderer);

  bool quit = false;
  //Speicher f�r die Datenstruktur des Spiels reservieren
  punkt *grid = malloc(sizeof(punkt) * GRID_HEIGHT * GRID_WIDTH);

  //Zur Initialisierung Fenster schwarz rendern. 
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);

  //Funktion zum Erzeugen des Spielfeldes aufrufen
  gridErzeugen(grid);

  //Datenstrukturen und Variablen zum Messen der Laufzeit
  struct timespec drawstart, drawende;
  struct timespec lnzstart, lnzende;
  struct timespec spielenstart, spielenende;
  struct timespec framestart, frameende;
  double drawelapsedTime, lnzelapsedTime, spielelapsedTime, frameTime;
  double drawaverage, lnzaverage, spielaverage, frameaverage;  
  int x = 0;
  float FPS = 0;

  //Main-Loop der Anwendung  
  while (!quit) {
    x++;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_KEY_DOWN) {
	//Programm mit ESC schlie�en
        if (event.key.key == SDLK_ESCAPE) {
          quit = true;
        }
	//Mit Taste R das Spiel neu starten
        if (event.key.key == SDLK_R) {
          gridErzeugen(grid);
        }
      }
      //Hook f�r Betriebssystem setzen um das Programm kontrolliert beenden zu k�nnen      
      if (event.type == SDL_EVENT_QUIT) {
        quit = true;
      }
    }

    //Setzen verschiedener Zeitstempel, dann Aufruf der einzelnen Funktionen des Spiels
    clock_gettime(CLOCK_MONOTONIC, &framestart);
    clock_gettime(CLOCK_MONOTONIC, &drawstart);
    //Visualisierungsfunktion aufrufen
    Draw(renderer, grid);
    clock_gettime(CLOCK_MONOTONIC, &drawende);
    drawelapsedTime = (drawende.tv_sec - drawstart.tv_sec) + (drawende.tv_nsec - drawstart.tv_nsec) / 1e9;
    drawaverage += drawelapsedTime;


    clock_gettime(CLOCK_MONOTONIC, &lnzstart);
    //Funktion zum Z�hlen der lebenden Nachbarn
    lebendeNachbarnZaehlen(grid);
    clock_gettime(CLOCK_MONOTONIC, &lnzende);
    lnzelapsedTime = (lnzende.tv_sec - lnzstart.tv_sec) + (lnzende.tv_nsec - lnzstart.tv_nsec) / 1e9;
    lnzaverage += lnzelapsedTime;

    clock_gettime(CLOCK_MONOTONIC, &spielenstart);
    //Funktion zum Festlegen des Status lebend/nichtlebend aufrufen
    Spielen(grid);
    clock_gettime(CLOCK_MONOTONIC, &spielenende);
    spielelapsedTime = (spielenende.tv_sec - spielenstart.tv_sec) + (spielenende.tv_nsec - spielenstart.tv_nsec) / 1e9;
    spielaverage += spielelapsedTime;


    clock_gettime(CLOCK_MONOTONIC, &frameende);
    frameTime = (frameende.tv_sec - framestart.tv_sec) + (frameende.tv_nsec - framestart.tv_nsec) / 1e9;
    frameaverage += frameTime;

    //Nach 100 Iterationen des Spiels die vergangene Zeit ausgeben
    if (x == 100) {
      printf("---------------------------------------------------------------------------\n");
      printf("Zeit f�r 100 Runden bei %d Threads: \n", NUM_THREADS);
      printf("Draw: %.3f s\n", drawaverage);
      drawaverage = 0;

      printf("LebendeNachbarnZaehlen: %.3f s\n", lnzaverage);
      lnzaverage = 0;

      printf("Spielen: %.3f s\n", spielaverage);
      spielaverage = 0;

      printf("Frame: %.3f s\n", frameaverage);
      FPS = 100 / frameaverage;
      frameaverage = 0;

      printf("Durchschnittliche FPS: %f\n", FPS);
      x = 0;    
    }
  }

  //Aufr�umen  
  free(grid);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();


  return 0;
}
