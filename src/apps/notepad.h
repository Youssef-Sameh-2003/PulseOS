#ifndef NOTEPAD_H
#define NOTEPAD_H

extern const char kbdus[128];
extern uint8_t inb(uint16_t port);

void notepad_app(void (*terminal_clear)(), void (*terminal_write)(const char*), int read_mode, const char* read_filename);

#endif