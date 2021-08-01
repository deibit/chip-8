#include <stdlib.h>

#ifndef CHIP8
#define CHIP8

size_t chip8_load(char *file);
void chip8_print_memory(size_t until);
void chip8_init(void);
uint8_t chip8_step(void);

#endif // CHIP8