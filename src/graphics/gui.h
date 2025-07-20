#ifndef GUI_H
#define GUI_H

#include <stdint.h>

// Simple framebuffer structure for 32bpp mode
typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t pitch;      // bytes per row
    uint32_t bpp;        // bits per pixel
    uint8_t *address;    // pointer to framebuffer
} framebuffer_t;

// Mouse state
typedef struct {
    int x, y;
    int buttons;
} mouse_t;

// GUI main entry point
void gui_main(framebuffer_t *fb);

#endif