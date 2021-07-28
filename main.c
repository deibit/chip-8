#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "SDL.h"
#include "chip8.h"

#define LEN(x) (sizeof(x) / sizeof(*x))

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 320
#define FPS (1000 / 60)

extern uint8_t display[64][32];
extern uint8_t keyboard[16];

int main(int argc, char **argv)
{
  if (SDL_Init(SDL_INIT_VIDEO) == 0)
  {
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    if (SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer) == 0)
    {
      SDL_bool done = SDL_FALSE;

      chip8_init();
      size_t loaded = chip8_load(argv[1]);
      //chip8_print_memory(loaded);
      //exit(0);

      while (!done)
      {
        SDL_Event event;
        Uint32 start = SDL_GetTicks();

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

        for (uint8_t x = 0; x < (SCREEN_WIDTH / 10); x++)
        {
          for (uint8_t y = 0; y < (SCREEN_HEIGHT / 10); y++)
          {
            if (display[x][y] != 0)
            {
              const SDL_Rect r = {.x = x * 10,
                                  .y = y * 10,
                                  .h = 10,
                                  .w = 10};
              SDL_RenderFillRect(
                  renderer, &r);
            }
          }
        }

        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&event))
        {
          if (event.type == SDL_QUIT)
          {
            done = SDL_TRUE;
          }
          else if (event.type == SDL_KEYDOWN)
          {
            switch (event.key.keysym.sym)
            {
            case SDL_SCANCODE_1:
              keyboard[0] = 1;
              break;
            case SDL_SCANCODE_2:
              keyboard[1] = 1;
              break;
            case SDL_SCANCODE_3:
              keyboard[2] = 1;
              break;
            case SDL_SCANCODE_4:
              keyboard[3] = 1;
              break;
            case SDL_SCANCODE_Q:
              keyboard[4] = 1;
              break;
            case SDL_SCANCODE_W:
              keyboard[5] = 1;
              break;
            case SDL_SCANCODE_E:
              keyboard[6] = 1;
              break;
            case SDL_SCANCODE_R:
              keyboard[7] = 1;
              break;
            case SDL_SCANCODE_A:
              keyboard[8] = 1;
              break;
            case SDL_SCANCODE_S:
              keyboard[9] = 1;
              break;
            case SDL_SCANCODE_D:
              keyboard[10] = 1;
              break;
            case SDL_SCANCODE_F:
              keyboard[11] = 1;
              break;
            case SDL_SCANCODE_Z:
              keyboard[12] = 1;
              break;
            case SDL_SCANCODE_X:
              keyboard[13] = 1;
              break;
            case SDL_SCANCODE_C:
              keyboard[14] = 1;
              break;
            case SDL_SCANCODE_V:
              keyboard[15] = 1;
              break;
            }
          }
          else if (event.type == SDL_KEYUP)
          {
            switch (event.key.keysym.sym)
            {
            case SDL_SCANCODE_1:
              keyboard[0] = 0;
              break;
            case SDL_SCANCODE_2:
              keyboard[1] = 0;
              break;
            case SDL_SCANCODE_3:
              keyboard[2] = 0;
              break;
            case SDL_SCANCODE_4:
              keyboard[3] = 0;
              break;
            case SDL_SCANCODE_Q:
              keyboard[4] = 0;
              break;
            case SDL_SCANCODE_W:
              keyboard[5] = 0;
              break;
            case SDL_SCANCODE_E:
              keyboard[6] = 0;
              break;
            case SDL_SCANCODE_R:
              keyboard[7] = 0;
              break;
            case SDL_SCANCODE_A:
              keyboard[8] = 0;
              break;
            case SDL_SCANCODE_S:
              keyboard[9] = 0;
              break;
            case SDL_SCANCODE_D:
              keyboard[10] = 0;
              break;
            case SDL_SCANCODE_F:
              keyboard[11] = 0;
              break;
            case SDL_SCANCODE_Z:
              keyboard[12] = 0;
              break;
            case SDL_SCANCODE_X:
              keyboard[13] = 0;
              break;
            case SDL_SCANCODE_C:
              keyboard[14] = 0;
              break;
            case SDL_SCANCODE_V:
              keyboard[15] = 0;
              break;
            }
          }
        }

        Uint32 diff = SDL_GetTicks() - start;
        if (diff < FPS)
        {
          SDL_Delay(FPS - diff);
        }
        chip8_step();
      }
    }

    if (renderer)
    {
      SDL_DestroyRenderer(renderer);
    }
    if (window)
    {
      SDL_DestroyWindow(window);
    }
  }
  SDL_Quit();
  return 0;
}