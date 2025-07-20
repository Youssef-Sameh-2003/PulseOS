// Minimal PS/2 mouse code (polling, platform-dependent).
#include <stdint.h>

// These functions should be implemented for your hardware.
// This example is for legacy PS/2 mouse (standard IO ports).

#define MOUSE_DATA_PORT 0x60
#define MOUSE_STATUS_PORT 0x64

static int mouse_cycle = 0;
static int mouse_bytes[3];

// Initialize mouse (send enable command)
void ps2_mouse_init() {
    // Enable auxiliary device
    outb(MOUSE_STATUS_PORT, 0xA8);
    // Enable mouse interrupts
    outb(MOUSE_STATUS_PORT, 0x20);
    uint8_t status = inb(MOUSE_DATA_PORT) | 2;
    outb(MOUSE_STATUS_PORT, 0x60);
    outb(MOUSE_DATA_PORT, status);
    // Tell mouse to use default settings
    outb(MOUSE_DATA_PORT, 0xF6);
    inb(MOUSE_DATA_PORT);
    // Enable mouse
    outb(MOUSE_DATA_PORT, 0xF4);
    inb(MOUSE_DATA_PORT);
}

// Poll mouse (returns 1 if movement detected)
int ps2_mouse_poll(int *dx, int *dy, int *buttons) {
    if (inb(MOUSE_STATUS_PORT) & 1) {
        uint8_t data = inb(MOUSE_DATA_PORT);
        mouse_bytes[mouse_cycle++] = data;
        if (mouse_cycle == 3) {
            mouse_cycle = 0;
            int x = (int)((int8_t)mouse_bytes[1]);
            int y = -(int)((int8_t)mouse_bytes[2]);
            *dx = x;
            *dy = y;
            *buttons = mouse_bytes[0] & 0x7;
            return 1;
        }
    }
    return 0;
}

// I/O functions
void outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}