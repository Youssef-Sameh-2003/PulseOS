#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>

typedef struct {
    uint32_t width, height, pitch, bpp;
    uint8_t *address;
} framebuffer_t;

int framebuffer_init(uint32_t mb2_addr, framebuffer_t *fb);

#endif