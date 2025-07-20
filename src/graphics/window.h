#ifndef WINDOW_H
#define WINDOW_H

#include <stdint.h>
#include "framebuffer.h"

typedef struct {
    uint32_t x, y, w, h;
    uint32_t bg_color;
    char title[32];
    int focused;
} window_t;

// Draw window
void draw_window(framebuffer_t *fb, const window_t *win);

#endif