#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>
#include "framebuffer.h"

typedef struct {
    int x, y;
    int buttons;
} mouse_t;

void mouse_draw_cursor(framebuffer_t *fb, mouse_t *mouse);

#endif