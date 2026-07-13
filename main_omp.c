#include <SDL3/SDL.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define TITLE "Conway's Game of Life"
#define WIDTH 1000
#define HEIGHT 1000
#define GRID_HEIGHT 10000
#define GRID_WIDTH 10000
#define DELAY 0

//Gr��e des Kamerafeldes
int CAMERA_GRID_X = 500;
int CAMERA_GRID_Y = 500;

int CAMERA_COORD_X = GRID_WIDTH / 2;
int CAMERA_COORD_Y = GRID_HEIGHT / 2;

Uint32 INIT_FLAGS = SDL_INIT_VIDEO;
Uint32 WINDOW_FLAGS = SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALWAYS_ON_TOP;

int currentheight = WIDTH;
int currentwidth = HEIGHT;
int weissrate = 5;
Uint8 r = 255;
Uint8 g = 255;
Uint8 b = 255;

typedef struct {
  bool weiss;
  int lebendeNachbarn;
} punkt;


void gridErzeugen(punkt *grid) {
  for (int i = 0; i < GRID_HEIGHT; i++) {
    for (int j = 0; j < GRID_WIDTH; j++) {
      punkt *elem = &grid[i * GRID_WIDTH + j];
      elem->weiss = false;
      if (rand() % weissrate == 0) {elem->weiss = true;}
    }
  } 
}

void Draw(SDL_Renderer *renderer, punkt *grid) {

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);  
  SDL_SetRenderDrawColor(renderer, r, g, b, 255);

  int anfang_x = CAMERA_COORD_X - (CAMERA_GRID_X / 2);
  int ende_x = CAMERA_COORD_X + (CAMERA_GRID_X / 2);
  int anfang_y = CAMERA_COORD_Y - (CAMERA_GRID_Y / 2);
  int ende_y = CAMERA_COORD_Y + (CAMERA_GRID_Y / 2);

  int punktbreite = currentwidth / CAMERA_GRID_X;
  int punkthohe = currentheight / CAMERA_GRID_Y;

  for (int i = anfang_y; i < ende_y; i++) {
    int aktuelly = (i - anfang_y) * punkthohe;

    for (int j = anfang_x; j < ende_x; j++) {
      punkt *elem = &grid[i * GRID_WIDTH + j];
      
      int aktuellx = (j - anfang_x) * punktbreite;
    
      SDL_FRect rect;
      rect.x = (float)aktuellx;
      rect.w = (float)punktbreite;
      rect.y = (float)aktuelly;
      rect.h = (float)punkthohe;
      
      if (elem->weiss) {
       	SDL_RenderFillRect(renderer, &rect);        
      }
    }  
  }
  SDL_RenderPresent(renderer);
  SDL_Delay(DELAY);
}


void lebendeNachbarnZaehlen(punkt *grid) {

#pragma omp parallel for schedule(static)
  for (int i = 0; i < GRID_HEIGHT; i++) {
    for (int j = 0; j < GRID_WIDTH; j++) {

      punkt *elem = &grid[i * GRID_WIDTH + j];
      int lebendeNachbarn = 0;
      
      for (int di = -1; di <= 1; di++) {
        for (int dj = -1; dj <= 1; dj++) {
	  if (di == 0 && dj == 0) continue;
	  int ni = i + di;
          int nj = j + dj;
          if (ni < 0 || ni >= GRID_HEIGHT || nj < 0 || nj >= GRID_WIDTH) continue;
          if (grid[ni * GRID_WIDTH + nj].weiss) ++lebendeNachbarn;
        }
      }        
      elem->lebendeNachbarn = lebendeNachbarn;
    }
  }
}




void Spielen(punkt *grid) {

#pragma omp parallel for schedule(static)
  for (int i = 0; i < GRID_HEIGHT; i++) {
    for (int j = 0; j < GRID_WIDTH; j++) {

      punkt *elem = &grid[i * GRID_WIDTH + j];
      
      if (elem->weiss) {
        if (elem->lebendeNachbarn == 2 || elem->lebendeNachbarn == 3) {
	  elem->weiss=true;
        } else {elem->weiss = false;}
      }
      
      if (!elem->weiss) {
        if (elem->lebendeNachbarn == 3) {
	  elem->weiss = true;
	} 
      }                 
    }
  }
}

int main() {

  srand((unsigned)time(0));
  
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


  while (!quit) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_KEY_DOWN) {
        switch (event.key.key) {
        case SDLK_ESCAPE:
          quit = true;
          break;
        case SDLK_W:
          CAMERA_COORD_Y -= 5;
          break;
        case SDLK_S:
          CAMERA_COORD_Y += 5;
          break;
        case SDLK_A:
          CAMERA_COORD_X -= 5;
          break;          
        case SDLK_D:
          CAMERA_COORD_X += 5;
          break;
        case SDLK_R:
          gridErzeugen(grid);
          break;	           
	}      
	if (event.type == SDL_EVENT_QUIT) {
	  quit = true;
	}        
      }
    }
      Draw(renderer, grid);
      lebendeNachbarnZaehlen(grid);  
      Spielen(grid);
    
  }
  
  free(grid);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0; 
}
