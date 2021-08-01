#include <stdlib.h>

#ifndef __CHIP8__
#define __CHIP8__

size_t chip8_load(char *file);
void chip8_print_memory(size_t until);
void chip8_init(void);
uint8_t chip8_step(void);

#endif // __CHIP8__