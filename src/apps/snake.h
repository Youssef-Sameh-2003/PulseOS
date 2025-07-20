#ifndef SNAKE_H
#define SNAKE_H

#include <stdint.h>

extern const char kbdus[128];
extern uint8_t inb(uint16_t port);

void snake_game(void (*terminal_clear)(), void (*terminal_write)(const char*));

#endif // SNAKE_H