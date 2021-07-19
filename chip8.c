#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "chip8.h"

#define LEN(x) (sizeof(x) / sizeof(*x))

static const uint8_t RATE = 60; // 60Hz

static uint8_t fonts[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

extern uint8_t display[64][32];

static uint8_t memory[0x1000]; // Memory 0x1000 8-bits addresses
static uint8_t V[0xF];         // Registers
static uint8_t IDX;            // Address register, also named 'i' or 'index'
static uint16_t PC = 0x200;    // Program counter
static uint8_t SP;             // Stack pointer
static uint8_t stack[0x100];   // 256 stack size
static uint8_t DT;             // Delay Timer
static uint8_t ST;             // Sound Timer

/*
 Load the game file content into memory
 program/data spans from 0x200 until 0xfff which gives 0xfff - 0x200 bytes
 */
size_t chip8_load(char *file)
{
  FILE *game = fopen(file, "rb");
  if (!game)
  {
    puts("Error loading game file\n");
    exit(EXIT_FAILURE);
  }

  size_t read = fread(&memory[0x200], sizeof(memory[0]), 0x1000 - 0x200, game);
  fprintf(stdout, "Read %lu bytes\n", read);
  fclose(game);

  return read;
}

/*
  Fetch one 16 bit instruction from 'memory' and return it
  Assume a Little Endian platform.
*/
static uint16_t fetch_at(uint16_t address)
{
  uint16_t word = *((uint16_t *)(memory + address));
  PC += 2; // Advance PC by two bytes (one instruction)
  return (word >> 8) | (word << 8);
}

/*
  Just fetch_at PC
*/
static uint16_t fetch()
{
  return fetch_at(PC);
}

/*
  Debug function to inspect WORDS in memory
*/
void chip8_print_memory(size_t until)
{
  for (size_t i = 0x200; i < (0x200 + until); i += 2)
  {
    printf("%04X\n", fetch_at(i));
  }
}

/*
  Initialize CPU and start
*/
void chip8_init(void)
{
  // Load fonts into memory
  memccpy(memory + 0x50, fonts, 0, LEN(fonts));
}
