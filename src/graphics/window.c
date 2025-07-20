#include "window.h"

void draw_rect(framebuffer_t *fb, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    for (uint32_t iy = y; iy < y + h; iy++)
        for (uint32_t ix = x; ix < x + w; ix++)
            fb_putpixel(fb, ix, iy, color);
}

void draw_window(framebuffer_t *fb, const window_t *win) {
    // Draw window background
    draw_rect(fb, win->x, win->y, win->w, win->h, win->bg_color);
    // Draw window border/title
    draw_rect(fb, win->x, win->y, win->w, 20, 0xCCCCCC); // title bar
    // TODO: draw text for title (font rendering)
    // TODO: highlight if focused
}