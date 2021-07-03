#include "SDL.h"
#include <stdlib.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 320;

typedef struct framebuffer_t
{
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Surface *surface;
} framebuffer_t;

framebuffer_t framebuffer;

/* Init framebuffer. Destroy resources and exit on failure */
void initFramebuffer()
{
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
  {
    fprintf(stderr, "Cannot initialize SDL: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  else
  {
    atexit(cleanup);
  }

  framebuffer.window =
      SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  if (framebuffer.window == NULL)
  {
    fprintf(stderr, "Cannot create a window: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  framebuffer.renderer =
      SDL_CreateRenderer(framebuffer.window, -1, SDL_RENDERER_ACCELERATED);
  if (framebuffer.renderer == NULL)
  {
    fprintf(stderr, "Cannot create a renderer: %s\n", SDL_GetError());
    SDL_DestroyWindow(framebuffer.window);
    exit(EXIT_FAILURE);
  }
}


void drawSprite(uint8_t x, uint8_t y, uint8_t height)
{
  SDL_SetRenderDrawColor(framebuffer.renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
  SDL_Rect rect;
  rect.x = x;
  rect.y = y;
  rect.w = 8;
  rect.h = height;
  SDL_RenderFillRect(framebuffer.renderer, &rect);
}

void testSDL()
{
  SDL_bool done = SDL_FALSE;
  while (!done)
  {
    SDL_Event event;

    SDL_SetRenderDrawColor(framebuffer.renderer, 0, 0, 0, 255);
    SDL_RenderClear(framebuffer.renderer);

    drawSprite(0, 0, 5);
    drawSprite(25, 25, 10);
    int x = rand() % 64;
    int y = rand() % 32;
    printf("%i\n", x);
    printf("%i\n", y);
    SDL_RenderDrawPoint(framebuffer.renderer, x, y);
    SDL_RenderDrawPoint(framebuffer.renderer, x + 2, y - 2);
    SDL_RenderDrawPoint(framebuffer.renderer, x + 2, y - 2);
    SDL_RenderPresent(framebuffer.renderer);

    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
      {
        done = SDL_TRUE;
      }
    }
  }
}

/* Clean resources and exit */
void cleanup()
{
  if (framebuffer.renderer != NULL)
  {
    SDL_DestroyRenderer(framebuffer.renderer);
  }
  if (framebuffer.window != NULL)
  {
    SDL_DestroyWindow(framebuffer.window);
  }
  SDL_Quit();
}



