#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "SDL.h"
#include "chip8.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 320

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
  chip8_init();
  size_t loaded = chip8_load(argv[1]);
  uint32_t speed = 1000 / 350;
  if (argc > 2)
  {
    speed = 1000 / atoi(argv[2]);
  }

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
      Uint32 *pixels = NULL;
      int pitch;
      Uint32 start = 0;
      Uint32 second = 0;
      Uint32 steps = 0;
      uint8_t do_draw = 0;

      while (!done)
      {
        do_draw = chip8_step();
        steps++;
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

        // Keymap https://massung.github.io/CHIP-8/
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
              keyboard[0x1] = val;
              break;
            case SDLK_2:
              keyboard[0x2] = val;
              break;
            case SDLK_3:
              keyboard[0x3] = val;
              break;
            case SDLK_4:
              keyboard[0xC] = val;
              break;
            case SDLK_q:
              keyboard[0x4] = val;
              break;
            case SDLK_w:
              keyboard[0x5] = val;
              break;
            case SDLK_e:
              keyboard[0x6] = val;
              break;
            case SDLK_r:
              keyboard[0xD] = val;
              break;
            case SDLK_a:
              keyboard[0x7] = val;
              break;
            case SDLK_s:
              keyboard[0x8] = val;
              break;
            case SDLK_d:
              keyboard[0x9] = val;
              break;
            case SDLK_f:
              keyboard[0xE] = val;
              break;
            case SDLK_z:
              keyboard[0xA] = val;
              break;
            case SDLK_x:
              keyboard[0x0] = val;
              break;
            case SDLK_c:
              keyboard[0xB] = val;
              break;
            case SDLK_v:
              keyboard[0xF] = val;
              break;
            default:
              break;
            }
          }
        }

        Uint32 diff = SDL_GetTicks() - start;
        if (diff < speed)
        {
          SDL_Delay(speed - diff);
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