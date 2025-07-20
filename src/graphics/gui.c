#include "gui.h"

static mouse_t mouse = {400, 300, 0};

static uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

static void fb_putpixel(framebuffer_t *fb, int x, int y, uint16_t color) {
    if (x < 0 || y < 0 || x >= (int)fb->width || y >= (int)fb->height) return;
    uint16_t *pixel = (uint16_t *)(fb->address + y * fb->pitch + x * 2);
    *pixel = color;
}

static void fb_clear(framebuffer_t *fb, uint16_t color) {
    for (uint32_t y = 0; y < fb->height; y++)
        for (uint32_t x = 0; x < fb->width; x++)
            fb_putpixel(fb, x, y, color);
}

static void fb_rect(framebuffer_t *fb, int x, int y, int w, int h, uint16_t color) {
    for (int iy = 0; iy < h; iy++)
        for (int ix = 0; ix < w; ix++)
            fb_putpixel(fb, x + ix, y + iy, color);
}

static void draw_mouse_cursor(framebuffer_t *fb, mouse_t *mouse) {
    for (int dx = -8; dx <= 8; dx++)
        fb_putpixel(fb, mouse->x + dx, mouse->y, rgb565(255, 0, 0));
    for (int dy = -8; dy <= 8; dy++)
        fb_putpixel(fb, mouse->x, mouse->y + dy, rgb565(255, 0, 0));
}

extern void ps2_mouse_init();
extern int ps2_mouse_poll(int *dx, int *dy, int *buttons);

void terminal_write_dec(uint16_t n) {
    char buf[12];
    int i = 10;
    buf[11] = 0;
    do {
        buf[i--] = '0' + (n % 10);
        n /= 10;
    } while(n && i >= 0);
    terminal_write(&buf[i + 1]);
}

void gui_main(framebuffer_t *fb) {
    ps2_mouse_init();
    fb_clear(fb, rgb565(40, 40, 40));
    
    int running = 1;
    int win_w = fb->width > 400 ? 400 : fb->width - 20;
    int win_h = fb->height > 300 ? 300 : fb->height - 20;
    int win_x = 10, win_y = 10;
    while (running) {
        int dx = 0, dy = 0, btns = 0;
        if (ps2_mouse_poll(&dx, &dy, &btns)) {
            mouse.x += dx;
            mouse.y += dy;
            if (mouse.x < 0) mouse.x = 0;
            if (mouse.y < 0) mouse.y = 0;
            if (mouse.x >= (int)fb->width) mouse.x = fb->width - 1;
            if (mouse.y >= (int)fb->height) mouse.y = fb->height - 1;
            mouse.buttons = btns;
        }
        fb_clear(fb, rgb565(40, 40, 40));
        fb_rect(fb, win_x, win_y, win_w, win_h, rgb565(220,220,255));
        fb_rect(fb, win_x, win_y, win_w, win_h > 24 ? 24 : win_h, rgb565(64,64,128));
        draw_mouse_cursor(fb, &mouse);
        int btn_x = win_x + 60, btn_y = win_y + win_h - 50;
        int btn_w = win_w > 120 ? 120 : win_w / 2;
        int btn_h = 40;
        fb_rect(fb, btn_x, btn_y, btn_w, btn_h, rgb565(180,255,180));
        if (mouse.x > btn_x && mouse.x < btn_x+btn_w &&
            mouse.y > btn_y && mouse.y < btn_y+btn_h && (mouse.buttons & 1)) {
            fb_rect(fb, btn_x, btn_y, btn_w, btn_h, rgb565(255,100,100));
        }
        for (volatile int i = 0; i < 100000; i++);
    }
}