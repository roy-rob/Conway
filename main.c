#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define TITLE "Conway's Game of Life"
#define WIDTH 1000
#define HEIGHT 1000
#define GRID_HEIGHT 250
#define GRID_WIDTH 250
#define DELAY 8

Uint32 INIT_FLAGS = SDL_INIT_VIDEO;
Uint32 WINDOW_FLAGS = SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALWAYS_ON_TOP;

int currentheight = WIDTH;
int currentwidth = HEIGHT;

typedef struct {
  SDL_FRect rect;
  bool weiss;
  int lebendeNachbarn;
} punkt;

void Draw(SDL_Renderer *renderer, punkt *grid) {

    for (int i = 0; i < GRID_HEIGHT; i++) {
      for (int j = 0; j < GRID_WIDTH; j++) {
        punkt *elem = &grid[i * GRID_WIDTH + j];
		  
        int r, g, b;
        if (elem->weiss) {
          r = 255;
          g = 255;
	  b = 255;
        } else {
          r = 0;
          g = 0;
	  b = 0;
        }
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
	SDL_RenderFillRect(renderer, &elem->rect);        
      }
    }    
    
    SDL_RenderPresent(renderer);
    SDL_Delay(DELAY);


  }


void Redraw(SDL_Window *window, punkt* grid) {
  
  SDL_GetWindowSize(window, &currentheight, &currentwidth);
  int punktbreite = currentwidth / GRID_WIDTH;
  int punkthöhe = currentheight / GRID_HEIGHT;
  
  for (int i = 0; i < GRID_HEIGHT; i++) {
    for (int j = 0; j < GRID_WIDTH; j++) {
      punkt *elem = &grid[i * GRID_WIDTH + j];
      int aktuellx = 0 + (j * punktbreite);
      int aktuelly = 0 + (i * punkthöhe);
      
      elem->rect.x = aktuellx;
      elem->rect.w = punktbreite;
      elem->rect.y = aktuelly;
      elem->rect.h = punkthöhe;
    }
  }
}

void lebendeNachbarnZaehlen(punkt *grid) {

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

int main(int argc, char **argv) {

  srand(time(0));
  
  SDL_Init(INIT_FLAGS);
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Event event;
  
  SDL_CreateWindowAndRenderer(TITLE, WIDTH, HEIGHT, WINDOW_FLAGS, &window, &renderer);

  bool quit = false;  
  punkt *grid = malloc(sizeof(punkt) * GRID_HEIGHT * GRID_WIDTH);

  int punktbreite = currentwidth / GRID_WIDTH;
  int punkthöhe = currentheight / GRID_HEIGHT;

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);
  
  for (int i = 0; i < GRID_HEIGHT; i++) {
    for (int j = 0; j < GRID_WIDTH; j++) {
      punkt *elem = &grid[i * GRID_WIDTH + j];
      int aktuellx = 0 + (j * punktbreite);
      int aktuelly = 0 + (i * punkthöhe);
      
      elem->rect.x = aktuellx;
      elem->rect.w = punktbreite;
      elem->rect.y = aktuelly;
      elem->rect.h = punkthöhe;

      elem->weiss = false;
      if (rand() % 5 == 0) {elem->weiss = true;}
    }
  }     

  while (!quit) {
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_KEY_DOWN) {
      if (event.key.key == SDLK_ESCAPE) {
	quit = true;
      }
    }      
    if (event.type == SDL_EVENT_QUIT) {
      quit = true;
      }        
    if (event.type == SDL_EVENT_WINDOW_RESIZED) {
      Redraw(window, grid);
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
