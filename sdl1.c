#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "SDL.h"

#define LEN(x) (sizeof(x) / sizeof(*x))

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 320
#define FPS (1000 / 60)

#define RANDOM_X rand() % (630 + 1 - 0) + 0
#define RANDOM_Y rand() % (310 + 1 - 0) + 0

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO) == 0)
    {
        SDL_Window *window = NULL;
        SDL_Renderer *renderer = NULL;

        if (SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer) == 0)
        {
            SDL_bool done = SDL_FALSE;
            SDL_Rect rects[64 * 32]; // Pixels in CHIP-8
            Uint32 rects_n = 0;

            while (!done)
            {
                SDL_Event event;
                Uint32 start = SDL_GetTicks();

                SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
                SDL_RenderClear(renderer);
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
                SDL_RenderFillRects(renderer, &rects[0], rects_n);
                SDL_RenderDrawRects(renderer, &rects[0], rects_n);
                SDL_RenderPresent(renderer);

                if (rects_n < 64 * 32)
                {
                    rects[rects_n++] = (SDL_Rect){.x = RANDOM_X, .y = RANDOM_Y, .h = 10, .w = 10};
                }

                assert(rects_n != 2048);

                while (SDL_PollEvent(&event))
                {
                    if (event.type == SDL_QUIT)
                    {
                        done = SDL_TRUE;
                    }
                }

                Uint32 diff = SDL_GetTicks() - start;
                if (diff < FPS)
                {
                    printf("%i\n", FPS - diff);
                    SDL_Delay(FPS - diff);
                }
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