#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Memory 0x1000 8-bits addresses
uint8_t memory[0x1000];

enum REGS {
    V0,
    V1,
    V2,
    V3,
    V4,
    V5,
    V6,
    V7,
    V8,
    V9,
    VA,
    VB,
    VC,
    VD,
    VE,
    VF,
    REGS_LENGTH
};

// Processor registers
typedef struct CPU
{
    uint8_t registers[REGS_LENGTH];

    uint16_t IDX; // Address register, also named 'i' or 'index'
    uint16_t PC;  // Program counter

} CPU;

CPU cpu;


/*
 * Load the game file content into memory
 * program/data spans from 0x200 until 0xfff which gives 0xfff - 0x200 bytes
 * */
size_t load(char *file)
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


int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    printf("Usage: chip8 <game.ch8>\n");
    return EXIT_FAILURE;
  }

  size_t read = load(argv[1]);
  for (int i = 0; i<read; i+=2) printf("%02X%02X\n", memory[i+0x200], memory[(i+1)+0x200]);

  cpu.PC = 0x200;

  return EXIT_SUCCESS;
}
