#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "SDL.h"
#include "chip8.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 320
#define FPS (1000 / 60)

extern uint8_t display[64][32];
extern uint8_t keyboard[16];

static void draw(Uint32 *pixels)
{
  for (uint8_t y = 0; y < 32; y++)
  {
    for (uint8_t x = 0; x < 64; x++)
    {
      Uint32 value = display[x][y] ? -1 : 0; // -1 -> white
      *pixels = value;
      pixels++;
    }
  }
}

int main(int argc, char **argv)
{
  if (SDL_Init(SDL_INIT_VIDEO) == 0)
  {
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture = NULL;

    window = SDL_CreateWindow("CHIP8",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              640, 320, SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                SDL_TEXTUREACCESS_STREAMING, 64, 32);

    if (window && renderer && texture)
    {
      SDL_bool done = SDL_FALSE;

      chip8_init();
      size_t loaded = chip8_load(argv[1]);
      //chip8_print_memory(loaded);

      Uint32 *pixels = NULL;
      int pitch;
      Uint32 start = 0;
      Uint32 second = 0;
      Uint32 steps = 0;
      uint8_t do_draw = 0;

      while (!done)
      {
        start = SDL_GetTicks();

        // Using texture instead of direct to renderer
        // https://github.com/danirod/chip8/blob/devel/src/chip8/libsdl.c#L217
        if (do_draw)
        {
          SDL_LockTexture(texture, NULL, (void **)&pixels, &pitch);
          draw(pixels);
          SDL_UnlockTexture(texture);

          SDL_RenderClear(renderer);
          SDL_RenderCopy(renderer, texture, NULL, NULL);
          SDL_RenderPresent(renderer);
          do_draw = 0;
        }

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
          if (event.type == SDL_QUIT)
            done = SDL_TRUE;
          else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
          {
            uint8_t val = event.type == SDL_KEYDOWN ? 1 : 0;
            switch (event.key.keysym.sym)
            {
            case SDLK_1:
              keyboard[0] = val;
              break;
            case SDLK_2:
              keyboard[1] = val;
              break;
            case SDLK_3:
              keyboard[2] = val;
              break;
            case SDLK_4:
              keyboard[3] = val;
              break;
            case SDLK_q:
              keyboard[4] = val;
              break;
            case SDLK_w:
              keyboard[5] = val;
              break;
            case SDLK_e:
              keyboard[6] = val;
              break;
            case SDLK_r:
              keyboard[7] = val;
              break;
            case SDLK_a:
              keyboard[8] = val;
              break;
            case SDLK_s:
              keyboard[9] = val;
              break;
            case SDLK_d:
              keyboard[10] = val;
              break;
            case SDLK_f:
              keyboard[11] = val;
              break;
            case SDLK_z:
              keyboard[12] = val;
              break;
            case SDLK_x:
              keyboard[13] = val;
              break;
            case SDLK_c:
              keyboard[14] = val;
              break;
            case SDLK_v:
              keyboard[15] = val;
              break;
            default:
              break;
            }
          }
        }

        do_draw = chip8_step();
        steps++;

        Uint32 diff = SDL_GetTicks() - start;
        if (diff < FPS)
        {
          SDL_Delay(FPS - diff);
        }

        second += diff;
        if (second > 1000)
        {
          printf("*** (second: %i) STEPS ---> %i\n", second, steps);
          steps = 0;
          second = 0;
        }
      }
    }

    if (texture)
    {
      SDL_DestroyTexture(texture);
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