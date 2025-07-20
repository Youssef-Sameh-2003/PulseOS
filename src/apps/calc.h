#ifndef CALC_H
#define CALC_H

#include <stdint.h>

extern const char kbdus[128];
extern uint8_t inb(uint16_t port);

void calc_app(void (*terminal_clear)(), void (*terminal_write)(const char*));

#endif