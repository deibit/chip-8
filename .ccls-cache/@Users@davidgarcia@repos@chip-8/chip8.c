#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "SDL.h"

// Memory 0x1000 8-bits addresses
int8_t memory[4096];

// Processor registers
typedef struct registers_t {
  int8_t V0;
  int8_t V1;
  int8_t V2;
  int8_t V3;
  int8_t V4;
  int8_t V5;
  int8_t V6;
  int8_t V7;
  int8_t V8;
  int8_t V9;
  int8_t VA;
  int8_t VB;
  int8_t VC;
  int8_t VD;
  int8_t VE;
  int8_t VF; // Special register: carry flag, no-borrow flag (substraction),
             // collision flag in draw ops

  uint16_t IDX; // Address register, also named 'i' or 'index'
  uint16_t PC;  // Program counter

} registers_t;

// CPU
typedef struct CPU_t {
  registers_t reg;
} CPU_t;

CPU_t CPU;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 320;

typedef struct framebuffer_t {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Surface *surface;
} framebuffer_t;

framebuffer_t framebuffer;

/*
 * Load the game file content into memory
 * program/data spans from 0x200 until 0xfff which gives 0xfff - 0x200 bytes
 * */
void load(char *file) {
  FILE *game = fopen(file, "rb");
  if (!game) {
    puts("Error loading game file\n");
    exit(EXIT_FAILURE);
  }

  size_t read = fread(&memory[0x200], sizeof(memory[0]), 0x1000 - 0x200, game);
  fprintf(stdout, "Readed %lu bytes\n", read);
  fclose(game);
}

/* Init framebuffer. Destroy resources and exit on failure */
void initFramebuffer() {
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    fprintf(stderr, "Cannot initialize SDL: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  } else {
    atexit(SDL_Quit);
  }

  framebuffer.window =
      SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  if (framebuffer.window == NULL) {
    fprintf(stderr, "Cannot create a window: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  framebuffer.renderer =
      SDL_CreateRenderer(framebuffer.window, -1, SDL_RENDERER_ACCELERATED);
  if (framebuffer.renderer == NULL) {
    fprintf(stderr, "Cannot create a renderer: %s\n", SDL_GetError());
    SDL_DestroyWindow(framebuffer.window);
    exit(EXIT_FAILURE);
  }
}

void drawSprite(uint8_t x, uint8_t y, uint8_t height) {
  SDL_SetRenderDrawColor(framebuffer.renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
  SDL_Rect rect;
  rect.x = x;
  rect.y = y;
  rect.w = 80;
  rect.h = height * 10;
  SDL_RenderFillRect(framebuffer.renderer, &rect);
}

void testSDL() {
  SDL_bool done = SDL_FALSE;
  while (!done) {
    SDL_Event event;

    SDL_SetRenderDrawColor(framebuffer.renderer, 0, 0, 0, 255);
    SDL_RenderClear(framebuffer.renderer);

    drawSprite(0, 0, 5);
    drawSprite(25, 25, 10);
    SDL_RenderDrawPoint(framebuffer.renderer, 300, 300);
    SDL_RenderPresent(framebuffer.renderer);

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        done = SDL_TRUE;
      }
    }
  }
}

/* Clean resources and exit */
void cleanup() {
  if (framebuffer.renderer != NULL) {
    SDL_DestroyRenderer(framebuffer.renderer);
  }
  if (framebuffer.window != NULL) {
    SDL_DestroyWindow(framebuffer.window);
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: chip8 <game.c8k>\n");
    return EXIT_FAILURE;
  }

  initFramebuffer();
  testSDL();
  load(argv[1]);

  CPU.reg.PC = 0x200;

  cleanup();
  return EXIT_SUCCESS;
}
