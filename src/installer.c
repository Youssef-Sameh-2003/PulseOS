#include <stddef.h>
#include <stdint.h>
#include "fs.h"
#include "disk/diskio.h"
#include "graphics/framebuffer.h"

extern void terminal_clear();
extern void terminal_write(const char *str);
extern uint8_t inb(uint16_t port);
extern const char kbdus[128];

// Simple boot sector (stub): JMP, OEM name, magic number
uint8_t boot_sector[512] = {
    0xEB,0x3C,0x90, 'P','u','l','s','e','O','S',' ',' ',' ',' ',' ',' ',
    // Zero padding...
    [510]=0x55, [511]=0xAA
};

extern uint8_t _binary_kernel_bin_start[];
extern uint8_t _binary_kernel_bin_end[];

void installer_run()
{
    terminal_clear();
    terminal_write("PulseOS Installer\n");
    terminal_write("This will erase and install PulseOS to your hard disk.\n");
    terminal_write("Press Y to continue, any other key to abort.\n");
    // Wait for user confirmation
    while (1)
    {
        if (inb(0x64) & 0x01)
        {
            uint8_t sc = inb(0x60);
            char c = kbdus[sc];
            if (!(sc & 0x80))
            {
                if (c == 'Y' || c == 'y')
                    break;
                else
                {
                    terminal_write("Aborted installation.\n");
                    return;
                }
            }
        }
    }
    terminal_write("Formatting disk...\n");
    disk_write(0, boot_sector, 1);

    terminal_write("Writing kernel...\n");
    size_t kernel_size = _binary_kernel_bin_end - _binary_kernel_bin_start;
    size_t sectors = (kernel_size + 511) / 512;
    disk_write(2, _binary_kernel_bin_start, sectors);

    terminal_write("Writing system files...\n");
    fs_init();
    fs_create("README.txt", NULL);
    fs_write("README.txt", NULL, "Welcome to PulseOS!\n", 20);

    terminal_write("Installation complete!\n");
    terminal_write("Please reboot and boot from your hard disk.\n");
    while (1) __asm__("hlt");
}