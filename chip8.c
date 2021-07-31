/*
    CHIP-8 emulator (David García - 2021)

    Interesting links:
      (guide)     https://tobiasvl.github.io/blog/write-a-chip-8-emulator/
      (test rom)  https://github.com/daniel5151/AC8E/tree/master/roms
      (reference) https://github.com/daniel5151/AC8E/blob/master/references/Cowgod's%20Chip-8%20Technical%20Reference.pdf
      (test rom)  https://github.com/corax89/chip8-test-rom
*/

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#include "chip8.h"

#define LEN(x) (sizeof(x) / sizeof(*x))
#define NNN(x) (x & 0x0FFF)
#define KK(x) ((x & 0x00FF))
#define OPCODE_TYPE(x) ((x & 0xF000) >> 12)
#define N(x) (x & 0x000F)
#define X(x) ((x & 0x0F00) >> 8)
#define Y(x) ((x & 0x00F0) >> 4)
#define MAX_STACK_SIZE 256

#define PROGRAM_START_ADDRESS 0x200
#define MAX_PROGRAM_MEMORY 0x1000 - 0x200

#define DEBUG 1

#if DEBUG
#define cdebug(...)               \
  do                              \
  {                               \
    fprintf(stderr, __VA_ARGS__); \
                                  \
  } while (0)
#else
#define cdebug(fmt, ...)
#endif

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

#define KEYBOARD_LENGTH 16

uint8_t display[64][32];
uint8_t keyboard[KEYBOARD_LENGTH];

static uint8_t memory[0x1000];              // Memory 0x1000 8-bits addresses
static uint8_t V[16];                       // Registers
static uint16_t I = 0;                      // Address register, also named 'i' or 'index'
static uint16_t PC = PROGRAM_START_ADDRESS; // Program counter
static uint16_t SP = 0;                     // Stack pointer
static uint16_t stack[MAX_STACK_SIZE];      // 256 stack size
static uint8_t DT = 0;                      // Delay Timer
static uint8_t ST = 0;                      // Sound Timer

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
  size_t read = fread(&memory[PROGRAM_START_ADDRESS], sizeof(memory[0]), MAX_PROGRAM_MEMORY, game);
  fprintf(stdout, "Read %lu bytes\n", read);
  fclose(game);

  return read;
}

/*
  Fetch one 16 bit opcode from 'memory' and return it
  Assume a Little Endian platform.
*/
static uint16_t fetch()
{
  uint16_t word = *((uint16_t *)(memory + PC));
  return (word >> 8) | (word << 8);
}

/*
  Another way to read instructions from memory (just to debug)
*/
static uint16_t get_memory_value_at(uint16_t address)
{
  uint16_t word = memory[address] << 8;
  word = word ^ memory[address + 1];
  return word;
}

/*
  Debug function to inspect WORDS in memory
  TODO: Make it private
*/
void chip8_print_memory(size_t until)
{
  for (uint16_t i = PROGRAM_START_ADDRESS; i < (PROGRAM_START_ADDRESS + until); i += 2)
  {
    printf("(%04X) %04X\n", i, get_memory_value_at(i));
  }
}

/*
  Zero display array
*/

static void clear_display()
{
  memset(display, 0, sizeof(uint8_t) * 64 * 32);
}

/*
  Calculates 8bit decimal -> BCD
*/
static void ascii2bcd(uint8_t arr[3], uint8_t num)
{
  arr[2] = num % 10;
  arr[1] = (num % 100 - arr[2]) / 10;
  arr[0] = (num - (arr[1] + arr[2])) / 100;
}

/*
  Debug CPU
*/

static void debug()
{
#if DEBUG
  const char *template = "-------------------------\nI: %04X\tPC: %X\tInst: %04X\tST: %X\tDT: %X\tSP: %X -> (%04X)\n";
  printf(template, I, PC, get_memory_value_at(PC), ST, DT, SP, stack[SP]);
  const char *regs1 = "V0: %x\tV1: %x\tV2: %x\tV3: %x\t";
  printf(regs1, V[0], V[1], V[2], V[3]);
  const char *regs2 = "V4: %x\tV5: %x\tV6: %x\tV7: %x\n";
  printf(regs2, V[4], V[5], V[6], V[7]);
  const char *regs3 = "V8: %x\tV9: %x\tVA: %x\tVB: %x\t";
  printf(regs3, V[8], V[9], V[0xa], V[0xb]);
  const char *regs4 = "VC: %x\tVD: %x\tVE: %x\tVF: %x\n\n";
  printf(regs4, V[0xc], V[0xd], V[0xe], V[0xf]);
#endif
}

/*
  Debug display
*/
static void debug_display()
{
#if DEBUG
  for (uint8_t y = 0; y < 32; y++)
  {
    for (uint8_t x = 0; x < 64; x++)
    {
      printf("%c", display[x][y] ? '@' : '.');
    }
    printf("\n");
  }
#endif
}

/*
  Debug keyboard
*/
static void debug_keyboard()
{
#if DEBUG
  for (uint8_t key = 0; key < KEYBOARD_LENGTH; key++)
  {
    printf("k(%x): %x\n", key, keyboard[key]);
  }
#endif
}

/*
  Initialize CPU and start
*/
void chip8_init(void)
{
  // Load fonts into memory
  memcpy(memory, fonts, LEN(fonts));
  // Clear display
  clear_display();
  // Initialize p-random number generator
  srand(time(0));
}

void chip8_step()
{
  uint16_t opcode = fetch();
  uint8_t type = OPCODE_TYPE(opcode);

  uint16_t nnn = NNN(opcode);
  uint8_t x = X(opcode);
  uint8_t y = Y(opcode);
  uint8_t kk = KK(opcode);
  uint8_t n = N(opcode);

  if (DT)
    DT--;
  if (ST)
    ST--;

  //debug();

  switch (type)
  {
  case 0x0:
    switch (opcode)
    {
    case 0x00E0: // Clear display
      clear_display();
      cdebug("CLS\n");
      PC += 2;
      break;

    case 0x00EE: // RET
      cdebug("RET\n");
      PC = stack[SP--];
      break;

    default:
      cdebug("Opps, unknown 0x0 type opcode: %X\n", opcode);
      exit(-1);
    }
    break;

  case 0x1: // JP nnn
    cdebug("JUMP (%X) -> (%X)\n", PC, nnn);
    PC = nnn;
    break;

  case 0x2: // CALL nnn
    stack[++SP] = PC + 2;
    PC = nnn;
    cdebug("CALL %X\n", PC);
    break;

  case 0x3: // SE Vx, byte
    PC += 2;
    cdebug("SKIP IF V(%X): (%X), EQUALS byte (%X)\n", x, V[x], kk);
    if (V[x] == kk)
    {
      PC += 2;
    }
    break;

  case 0x4: // SNE Vx, byte
    PC += 2;
    cdebug("SKIP IF V(%X): (%X) IS NOT %X\n", x, V[x], kk);
    if (V[x] != kk)
    {
      PC += 2;
    }
    break;

  case 0x5: // SE Vx, Vy
    PC += 2;
    if (V[x] == V[y])
    {
      cdebug("SKIP IF V(%X): %X IS EQUAL V(%X): %X\n", x, V[x], y, V[y]);
      PC += 2;
    }

    break;

  case 0x6: // LD Vx, byte
    cdebug("LOAD V(%x) <- (%X)\n", x, kk);
    V[x] = kk;
    PC += 2;
    break;

  case 0x7: // ADD Vx, byte
    cdebug("ADD V(%X) <- %X\n", x, kk);
    V[x] += kk;
    PC += 2;
    break;

  case 0x8:
    switch (n)
    {
    case 0: // LD Vx, Vy
      cdebug("LOAD V(%X) = V(%X): %X\n", x, y, V[y]);
      V[x] = V[y];
      PC += 2;
      break;

    case 1: // OR Vx, Vy
      cdebug("OR V(%X) |= V(%X)\n", x, y);
      V[x] = V[x] | V[y];
      PC += 2;
      break;

    case 2: // AND Vx, Vy
      cdebug("AND V(%X) &= V(%X)\n", x, y);
      V[x] = V[x] & V[y];
      PC += 2;
      break;

    case 3: // XOR Vx, Vy
      cdebug("XOR V(%X) ^= V(%X)\n", x, y);
      V[x] = V[x] ^ V[y];
      PC += 2;
      break;

    case 4: // ADD Vx, Vy
    {
      cdebug("ADD Vx(%X) + Vy(%X) (Vf = 1 IF Vx > Vy)\n", x, y);

      V[0xF] = V[x] > (UINT8_MAX - V[y]) ? 1 : 0;
      V[x] = V[x] + V[y];

      PC += 2;
    }
    break;

    case 5: // SUB Vx, Vy
    {
      cdebug("SUB V(%X) - V(%X) (Vf = 1 IF V(x) > V(y))\n", x, y);

      V[0xF] = (V[x] > V[y]) ? 1 : 0;
      V[x] = V[x] - V[y];

      PC += 2;
    }
    break;

    case 6: // SHR Vx {, Vy}
    {
      cdebug("SHR (/2) Vx(%X). IF LSB == 1; Vf = 1 ELSE Vf = 0\n", x);

      V[0xF] = V[x] & 0x1;
      V[x] = (V[x] >> 1);

      PC += 2;
    }
    break;

    case 7: // SUBN Vx, Vy
    {
      cdebug("SUBN Vx(%X) = Vy(%X) - Vx(%X) (IF Vx > Vy -> Vf = 1 ELSE 0)\n", x, y, x);

      V[0xF] = (V[y] > V[x]) ? 1 : 0;
      V[x] = V[y] - V[x];

      PC += 2;
    }
    break;

    case 0xE: //SHL Vx {, Vy}
    {
      cdebug("SHL (*2) Vx(%X) *= 2. IF MSB == 1; Vf = 1 ELSE Vf = 0\n", x);

      V[0xF] = (V[x] >> 7) & 0x1;
      V[x] = (V[x] << 1);

      PC += 2;
    }
    break;
    default:
      cdebug("Opps, unknown 0x8 type opcode: %X\n", opcode);
      exit(-1);
    }
    break;

  case 0x9: // SNE Vx, Vy
    cdebug("SKIP IF NOT EQUAL. Vx(%X) != Vy(%X)\n", x, y);
    if (V[x] != V[y])
    {
      PC += 2;
    }
    PC += 2;
    break;

  case 0xA: // LD I, addr
    cdebug("LOAD (I) -> %X\n", nnn);
    I = nnn;
    PC += 2;
    break;

  case 0xB: // JP V0, addr
    cdebug("JUMP (%X) + (%X) = (%X)\n", PC, nnn, V[0]);
    PC = nnn + V[0];
    break;

  case 0xC: // RND Vx, byte
    cdebug("RND V(%X) -> rnd & %X\n", x, kk);
    V[x] = (rand() % 0x256) & kk;
    PC += 2;
    break;

  case 0xD: // DRW Vx, Vy, nibble
  {
    //clear_display();

    const uint8_t BITS[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
    V[0xF] = 0;

    cdebug("DRAW at X: %u Y: %u FROM: %X, %X bytes\n", V[x], V[y], I, n);

    uint8_t col = V[x];
    uint8_t row = V[y];

    for (uint8_t i = 0; i < n; i++) // read 'n' bytes
    {
      uint8_t byte = memory[I + i]; // read current byte

      for (int pixel = 0; pixel < 8; pixel++) // xor pixels
      {
        uint8_t new = (byte >> pixel) & 0x1;
        uint8_t *old = &display[(col + 7 - pixel) % 64][row % 32];

        if (*old && new) // We shall turn off a pixel
        {
          V[0xF] = 1;
        }

        *old = *old ^ new; // XOR'ed pixel
      }
      row++; // Jump to another row
    }
    //debug_display();
    PC += 2;
  }
  break;

  case 0xE:
    switch (n)
    {
    case 0xE: // SKP Vx
      if (keyboard[x] == 1)
      {
        cdebug("\033[0;31mKEY DOWN: %X\n\033[0m", keyboard[x]);
        PC += 2;
      }
      PC += 2;
      break;
    case 0x1: // SKNP Vx
      if (keyboard[x] == 0)
      {
        cdebug("\033[0;31mKEY UP: %X\n\033[0m", keyboard[x]);
        PC += 2;
      }
      PC += 2;
      break;
    default:
      cdebug("Opps, unknown 0xE type opcode: %X\n", opcode);
      exit(-1);
    }
    break;

  case 0xF:
    switch (kk)
    {
    case 0x07: // LD Vx, DT
      V[x] = DT;
      cdebug("SET Vx(%X) <- DELAY %X\n", x, DT);
      PC += 2;
      break;

    case 0x0A: // LD Vx, K
      // Wait until a key is pressed. We don't advance PC until any('keyboard') == 1
      for (uint8_t i = 0; i < 0xF; i++)
      {
        if (keyboard[i] == 1)
        {
          cdebug("WAIT PRESSED...OK\n");
          PC += 2;
        }
        break;
      }
      break;

    case 0x15: // LD DT, Vx
      cdebug("SET DELAY %X -> Vx(%X): %X\n", DT, x, V[x]);
      DT = V[x];
      PC += 2;
      break;

    case 0x18: // LD ST, Vx
      cdebug("SET SOUND %X -> Vx(%X): %X\n", ST, x, V[x]);
      ST = V[x];
      PC += 2;
      break;

    case 0x1E: // ADD I, Vx
      cdebug("ADD I -> Vx(%X): %X\n", x, V[x]);
      V[0xF] = (I + V[x] > 0xfff) ? 1 : 0;
      I += V[x];
      PC += 2;
      break;

    case 0x29: // LD F, Vx
      cdebug("LOAD FONT (%X)\n", V[x]);
      I = V[x] * 5;
      PC += 2;
      break;

    case 0x33: // LD B, Vx
    {
      uint8_t value = V[x];
      uint8_t arr[3];
      ascii2bcd(arr, value);
      memory[I] = arr[0];
      memory[I + 1] = arr[1];
      memory[I + 2] = arr[2];
      cdebug("BCD OF %X IS -> I: (%X), I+1: (%X), I+2: (%X)\n", value, arr[0], arr[1], arr[2]);
      PC += 2;
    }
    break;

    case 0x55: // LD [I], Vx
      cdebug("STORE REGISTERS\n");
      for (uint8_t i = 0; i <= x; i++)
      {
        memory[I + i] = V[i];
      }
      PC += 2;
      break;

    case 0x65: // LD Vx, [I]
      cdebug("RESTORE REGISTERS\n");
      for (uint8_t i = 0; i <= x; i++)
      {
        V[i] = memory[I + i];
      }
      PC += 2;
      break;
    default:
      cdebug("Opps, unknown 0xF type opcode: %X\n", opcode);
      exit(-1);
    }
    //PC += 2;
    break;

  default:
    fprintf(stderr, "Oops! Unknown opcode type: %x\n", opcode);
    exit(-1);
  }
}
