#include "framebuffer.h"

#define MULTIBOOT2_TAG_FRAMEBUFFER 8

int framebuffer_init(uint32_t mb2_addr, framebuffer_t *fb) {
    uint32_t size = *(uint32_t *)mb2_addr;
    uint32_t tag_addr = mb2_addr + 8;
    while (tag_addr < mb2_addr + size) {
        uint32_t type = *(uint32_t *)tag_addr;
        uint32_t tagsize = *(uint32_t *)(tag_addr + 4);
        if (type == MULTIBOOT2_TAG_FRAMEBUFFER) {
            uint64_t addr = *(uint64_t *)(tag_addr + 8);
            fb->address = (uint8_t *)(uintptr_t)addr;
            fb->width   = *(uint32_t *)(tag_addr + 16);
            fb->height  = *(uint32_t *)(tag_addr + 20);
            fb->pitch   = *(uint32_t *)(tag_addr + 24);
            fb->bpp     = *(uint8_t *)(tag_addr + 28);
            return 0;
        }
        tag_addr += ((tagsize + 7) & ~7);
    }
    return -1;
}