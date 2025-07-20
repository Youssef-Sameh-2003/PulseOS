#include <stdint.h>

// Print a single character to the terminal (example for VGA text mode)
void print_char(char c) {
    volatile char *video = (volatile char *)0xB8000;
    static uint16_t cursor = 0;
    if (c == '\n') {
        cursor += (80 - (cursor % 80));
    } else {
        video[cursor * 2] = c;
        video[cursor * 2 + 1] = 0x07; // light grey
        cursor++;
    }
    // Optionally: handle scrolling, etc.
}

// Print a byte as two hex digits
void print_hex(uint8_t value) {
    char hex[] = "0123456789ABCDEF";
    print_char(hex[(value >> 4) & 0xF]);
    print_char(hex[value & 0xF]);
}