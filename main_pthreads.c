#include <SDL3/SDL.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <pthread.h>

#define TITLE "Conway's Game of Life"
#define WIDTH 1000
#define HEIGHT 1000
#define GRID_HEIGHT 1000
#define GRID_WIDTH 1000
#define DELAY 0

#define NUM_THREADS 12

Uint32 INIT_FLAGS = SDL_INIT_VIDEO;
Uint32 WINDOW_FLAGS = SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALWAYS_ON_TOP;

int currentheight = WIDTH;
int currentwidth = HEIGHT;
int weissrate = 5;
Uint8 r = 255;
Uint8 g = 255;
Uint8 b = 255;

typedef struct {
  SDL_FRect rect;
  bool weiss;
  int lebendeNachbarn;
} punkt;

void gridErzeugen(punkt *grid) {
  int punktbreite = currentwidth / GRID_WIDTH;
  int punkthohe = currentheight / GRID_HEIGHT;

    for (int i = 0; i < GRID_HEIGHT; i++) {
      for (int j = 0; j < GRID_WIDTH; j++) {
        punkt *elem = &grid[i * GRID_WIDTH + j];

        elem->rect.x = j * punktbreite;
        elem->rect.y = i * punkthohe;
        elem->rect.w = punktbreite;
        elem->rect.h = punkthohe;

        elem->weiss = rand() % weissrate == 0;
      }
    }
  }

  
void Draw(SDL_Renderer *renderer, punkt *grid) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);
  SDL_SetRenderDrawColor(renderer, r, g, b, 255);

  for (int i = 0; i < GRID_HEIGHT; i++) {
    for (int j = 0; j < GRID_WIDTH; j++) {
      punkt *elem = &grid[i * GRID_WIDTH + j];

      if (elem->weiss) {
        SDL_RenderFillRect(renderer, &elem->rect);
      }
    }
  }
  SDL_RenderPresent(renderer);
  SDL_Delay(DELAY);
}

void Redraw(SDL_Window *window, punkt *grid) {
  SDL_GetWindowSize(window, &currentheight, &currentwidth);

  int punktbreite = currentwidth / GRID_WIDTH;
  int punkthohe = currentheight / GRID_HEIGHT;

  for (int i = 0; i < GRID_HEIGHT; i++) {
    for (int j = 0; j < GRID_WIDTH; j++) {
      punkt *elem = &grid[i * GRID_WIDTH + j];

      elem->rect.x = j * punktbreite;
      elem->rect.y = i * punkthohe;
      elem->rect.w = punktbreite;
      elem->rect.h = punkthohe;
    }
  }
}

typedef struct {
  punkt *grid;
  int startRow;
  int endRow;
} ThreadData;


void *countNeighbours(void *arg) {
  ThreadData *data = (ThreadData *)arg;

  for (int i = data->startRow; i < data->endRow; i++) {
    for (int j = 0; j < GRID_WIDTH; j++) {
      punkt *elem = &data->grid[i * GRID_WIDTH + j];
      int lebendeNachbarn = 0;

      for (int di = -1; di <= 1; di++) {
	for (int dj = -1; dj <= 1; dj++) {
	  if (di == 0 && dj == 0) continue;
	  int ni = i + di;
	  int nj = j + dj;

	  if (ni < 0 || ni >= GRID_HEIGHT || nj < 0 || nj >= GRID_WIDTH) continue;
	  if (data->grid[ni * GRID_WIDTH + nj].weiss) lebendeNachbarn++;
	}
      }
      elem->lebendeNachbarn = lebendeNachbarn;
    }
  }

  return NULL;
}


void lebendeNachbarnZaehlen(punkt *grid) {
  pthread_t threads[NUM_THREADS];
  ThreadData data[NUM_THREADS];

  int rowsPerThread = GRID_HEIGHT / NUM_THREADS;

  for (int i = 0; i < NUM_THREADS; i++) {

    data[i].grid = grid;
    data[i].startRow = i * rowsPerThread;

    if (i == NUM_THREADS - 1) data[i].endRow = GRID_HEIGHT;
    else data[i].endRow = (i + 1) * rowsPerThread;

    pthread_create(&threads[i], NULL, countNeighbours, &data[i]);
  }

  for (int i = 0; i < NUM_THREADS; i++) pthread_join(threads[i], NULL);
}


void Spielen(punkt *grid) {
  for (int i = 0; i < GRID_HEIGHT; i++) {
    for (int j = 0; j < GRID_WIDTH; j++) {
      punkt *elem = &grid[i * GRID_WIDTH + j];

      if (elem->weiss) {
        if (elem->lebendeNachbarn == 2 || elem->lebendeNachbarn == 3) elem->weiss = true;
        else elem->weiss = false;
      } else {
        if (elem->lebendeNachbarn == 3) elem->weiss = true;
      }
    }
  }
}

int main() {
  srand((unsigned)time(0));
  srand(1);

  SDL_Init(INIT_FLAGS);
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Event event;

  SDL_CreateWindowAndRenderer(TITLE, WIDTH, HEIGHT, WINDOW_FLAGS, &window, &renderer);

  bool quit = false;
  punkt *grid = malloc(sizeof(punkt) * GRID_HEIGHT * GRID_WIDTH);

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);

  gridErzeugen(grid);

  struct timespec start, ende;
  clock_gettime(CLOCK_MONOTONIC, &start);
  int x = 0;
  while (!quit) {
    x++;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_KEY_DOWN) {
        if (event.key.key == SDLK_ESCAPE) {
          quit = true;
        }
        if (event.key.key == SDLK_R) {
          gridErzeugen(grid);
        }
      }
      if (event.type == SDL_EVENT_QUIT) {
        quit = true;
      }
      if (event.type == SDL_EVENT_WINDOW_RESIZED) {
        Redraw(window, grid);
      }
    }
    if (x%1 == 0) {
      Draw(renderer, grid);
    }
    lebendeNachbarnZaehlen(grid);
    Spielen(grid);

    if (x == 100) {
      clock_gettime(CLOCK_MONOTONIC, &ende);
      double elapsedTime = (ende.tv_sec - start.tv_sec) + (ende.tv_nsec - start.tv_nsec) / 1e9;
      printf("%.3f\n", elapsedTime);

      // for (int i = 0; i < GRID_HEIGHT; i++) {
      //   for (int j = 0; j < GRID_WIDTH; j++) {
      //     punkt *elem = &grid[i * GRID_WIDTH + j];
      //     printf("%B%d", elem->weiss, elem->lebendeNachbarn);
      //   }
      // }
    }
  }

  free(grid);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();


  return 0;
}

