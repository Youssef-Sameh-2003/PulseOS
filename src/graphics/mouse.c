#include "mouse.h"

void mouse_draw_cursor(framebuffer_t *fb, mouse_t *mouse) {
    // Simple crosshair
    for (int dx = -5; dx <= 5; dx++)
        fb_putpixel(fb, mouse->x + dx, mouse->y, 0xFF0000);
    for (int dy = -5; dy <= 5; dy++)
        fb_putpixel(fb, mouse->x, mouse->y + dy, 0xFF0000);
}