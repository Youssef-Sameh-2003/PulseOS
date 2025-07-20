#include <stddef.h>
#include <stdint.h>
#include "apps/snake.h"
#include "apps/calc.h"
#include "apps/notepad.h"
#include "graphics/framebuffer.h"
#include "fs.h"
#include "net/wifi.h"
// ============ VGA Terminal =============

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((volatile uint16_t *)0xB8000)

static size_t terminal_row = 0;
static size_t terminal_col = 0;
static uint8_t terminal_color = 0x0F; // White on black

void terminal_clear()
{
    for (size_t y = 0; y < VGA_HEIGHT; y++)
    {
        for (size_t x = 0; x < VGA_WIDTH; x++)
        {
            VGA_MEMORY[y * VGA_WIDTH + x] = (terminal_color << 8) | ' ';
        }
    }
    terminal_row = 0;
    terminal_col = 0;
}

void terminal_setcolor(uint8_t color)
{
    terminal_color = color;
}

void terminal_putchar(char c)
{
    if (c == '\n')
    {
        terminal_row++;
        terminal_col = 0;
    }
    else if (c == '\b')
    {
        if (terminal_col > 0)
        {
            terminal_col--;
            VGA_MEMORY[terminal_row * VGA_WIDTH + terminal_col] = (terminal_color << 8) | ' ';
        }
    }
    else
    {
        VGA_MEMORY[terminal_row * VGA_WIDTH + terminal_col] = (terminal_color << 8) | c;
        terminal_col++;
        if (terminal_col >= VGA_WIDTH)
        {
            terminal_col = 0;
            terminal_row++;
        }
    }
    if (terminal_row >= VGA_HEIGHT)
    {
        terminal_clear();
    }
}

void terminal_write(const char *str)
{
    while (*str)
    {
        terminal_putchar(*str++);
    }
}

void prompt()
{
    terminal_write("\n$ ");
}

// ============ Keyboard Tables =============

const char kbdus[128] = {
    /* 0x00 */ 0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    /* 0x0F */ '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    /* 0x1D */ 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    /* 0x2A */ 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0,
    /* 0x39 - 0x7F */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

const char kbdus_shift[128] = {
    /* 0x00 */ 0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    /* 0x0F */ '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    /* 0x1D */ 0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    /* 0x2A */ 0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0,
    /* 0x39 - 0x7F */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

volatile int shift_pressed = 0;

// ============ Port I/O =============

uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// ============ String functions =============

int strcmp(const char *a, const char *b)
{
    while (*a && *b && *a == *b)
    {
        a++;
        b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
    while (n && *s1 && (*s1 == *s2))
    {
        ++s1;
        ++s2;
        --n;
    }
    if (n == 0)
    {
        return 0;
    }
    return (*(unsigned char *)s1) - (*(unsigned char *)s2);
}

// ============ Command Processing =============

#define CMD_BUF_SIZE 128
static char cmd_buffer[CMD_BUF_SIZE];
static size_t cmd_len = 0;

void process_command(const char *cmd)
{
    if (!cmd || !cmd[0])
        return;
    if (!strcmp(cmd, "clear"))
    {
        terminal_clear();
        prompt();
    }
    else if (!strcmp(cmd, "help"))
    {
        terminal_write("\nAvailable commands:\n");
        terminal_write("  help        - Show this help\n");
        terminal_write("  clear       - Clear screen\n");
        terminal_write("  about       - System info\n");
        terminal_write("  run <app>   - Run an application\n");
        terminal_write("\nAvailable apps: snake, calc, notepad\n");
        prompt();
    }
    else if (!strcmp(cmd, "about"))
    {
        terminal_write("\nPulseOS Terminal\n");
        terminal_write("Author: Youssef Sameh\n");
        terminal_write("Version: 0.2\n");
        prompt();
    }
    else if (!strcmp(cmd, "ls"))
    {
        char out[1024];
        fs_listdir(cmd, out, sizeof(out));
        terminal_write("\nFiles & Dirs:\n");
        terminal_write(out);
        prompt();
    }
    
    else if (!strncmp(cmd, "mkdir ", 6))
    {
        const char *arg = cmd + 6;
        if (fs_mkdir(cmd, arg) == 0)
            terminal_write("\nDirectory created.\n");
        else
            terminal_write("\nFailed to create directory.\n");
        prompt();
    }
    else if (!strncmp(cmd, "run ", 4))
    {
        // Parse: run <app> [options]
        const char *arg = cmd + 4;
        // Extract app name
        char appname[32] = {0};
        int i = 0;
        while (arg[i] && arg[i] != ' ' && i < 31)
        {
            appname[i] = arg[i];
            i++;
        }
        appname[i] = 0;
        const char *opt = arg + i;
        if (!strcmp(appname, "snake"))
        {
            snake_game(terminal_clear, terminal_write);
            prompt();
        }
        else if (!strcmp(appname, "calc"))
        {
            calc_app(terminal_clear, terminal_write);
            prompt();
        }
        else if (!strcmp(appname, "notepad"))
        {
            int read_mode = 0;
            char filename[FS_MAX_FILENAME] = {0};
            // Check for option -r filename
            while (*opt == ' ')
                opt++;
            if (*opt == '-' && *(opt + 1) == 'r')
            {
                opt += 2;
                while (*opt == ' ')
                    opt++;
                int j = 0;
                while (opt[j] && opt[j] != ' ' && j < FS_MAX_FILENAME - 1)
                {
                    filename[j] = opt[j];
                    j++;
                }
                filename[j] = 0;
                if (filename[0])
                    read_mode = 1;
            }
            notepad_app(terminal_clear, terminal_write, read_mode, filename);
            prompt();
        }
        else
        {
            terminal_write("\nUnknown app. Type 'help' for available apps.\n");
            prompt();
        }
    }
    else
    {
        terminal_write("\nUnknown command. Type 'help'.\n");
        prompt();
    }
}

// ============ Main Input Loop (Shift support) =============
void print_mac(const uint8_t *mac)
{
    for (int i = 0; i < 6; ++i)
    {
        print_hex(mac[i]); // Replace with your own print function
        print_char(i < 5 ? ':' : '\n');
    }
}

void splash_screen()
{
    terminal_clear();
    terminal_setcolor(0x1F); // Bright white on blue, for example
    terminal_write("\n\n\n\n\n");
    terminal_write("        ***********************\n");
    terminal_write("        *     PulseOS v0.2     *\n");
    terminal_write("        *  by Youssef Sameh    *\n");
    terminal_write("        ***********************\n");
    terminal_write("\n\n      Loading...");
}

void sleep_5_seconds()
{
    // Calibrate this for your hardware; example for ~5 seconds:
    for (volatile unsigned long i = 0; i < 200000000; ++i)
    {
        __asm__ volatile("nop");
    }
}

void main_input_loop()
{
    prompt();
    cmd_len = 0;
    while (1)
    {
        if (inb(0x64) & 0x01)
        {
            uint8_t sc = inb(0x60);

            // Shift key logic
            if (sc == 0x2A || sc == 0x36)
            { // Shift press
                shift_pressed = 1;
                continue;
            }
            if (sc == 0xAA || sc == 0xB6)
            { // Shift release
                shift_pressed = 0;
                continue;
            }

            if (!(sc & 0x80))
            { // Key press
                char c = shift_pressed ? kbdus_shift[sc] : kbdus[sc];
                if (c == '\b' && cmd_len > 0)
                {
                    cmd_len--;
                    terminal_putchar('\b');
                }
                else if (c == '\n')
                {
                    cmd_buffer[cmd_len] = '\0';
                    terminal_write("\n");
                    process_command(cmd_buffer);
                    cmd_len = 0;
                }
                else if (c && cmd_len < CMD_BUF_SIZE - 1)
                {
                    cmd_buffer[cmd_len++] = c;
                    char out[2] = {c, '\0'};
                    terminal_write(out);
                }
            }
        }
        for (volatile int i = 0; i < 50000; i++)
            ; // Small delay
    }
}

// ============ Kernel Entry Point =============

void kernel_main(uint32_t mb2_addr)
{
    splash_screen();
    sleep_5_seconds();
    terminal_clear();
    terminal_setcolor(0x0F);
    terminal_write("PulseOS Boot\n");
    terminal_write("=======================\n");
    terminal_write("Select mode:\n");
    terminal_write("1. Terminal\n");
    terminal_write("2. GUI (experimental)\n");
    terminal_write("3. Install OS to disk\n");
    terminal_write("Enter choice [1/2/3]: ");

    // Wait for user input (single key)

    int choice = 0;
    while (1)
    {
        if (inb(0x64) & 0x01)
        {
            uint8_t sc = inb(0x60);
            char c = kbdus[sc];
            if (!(sc & 0x80) && (c == '1' || c == '2' || c == '3'))
            {
                choice = c - '0';
                char out[4] = {c, '\n', 0};
                terminal_write(out);
                break;
            }
        }
        for (volatile int i = 0; i < 50000; i++)
            ;
    }
    if (choice == 3)
    {
        terminal_write("Launching Installer...\n");
        installer_run();
    }

    else if (choice == 2)
    {
        // Experimental GUI mode
        terminal_clear();
        terminal_write("Launching GUI...\n");
        framebuffer_t fb;
        if (framebuffer_init(mb2_addr, &fb) == 0)
        {
            terminal_write("FB info:\n");
            terminal_write("width: ");
            {
                char buf[16];
                int n = fb.width, i = 15;
                buf[15] = 0;
                do
                {
                    buf[--i] = '0' + (n % 10);
                    n /= 10;
                } while (n && i > 0);
                terminal_write(&buf[i]);
                terminal_write("\n");
            }
            terminal_write("height: ");
            {
                char buf[16];
                int n = fb.height, i = 15;
                buf[15] = 0;
                do
                {
                    buf[--i] = '0' + (n % 10);
                    n /= 10;
                } while (n && i > 0);
                terminal_write(&buf[i]);
                terminal_write("\n");
            }
            terminal_write("bpp: ");
            {
                char buf[4];
                int n = fb.bpp, i = 3;
                buf[3] = 0;
                do
                {
                    buf[--i] = '0' + (n % 10);
                    n /= 10;
                } while (n && i > 0);
                terminal_write(&buf[i]);
                terminal_write("\n");
            }
            terminal_write("addr: ");
            {
                char buf[16];
                uintptr_t n = (uintptr_t)fb.address;
                int i = 15;
                buf[15] = 0;
                do
                {
                    buf[--i] = "0123456789ABCDEF"[n % 16];
                    n /= 16;
                } while (n && i > 0);
                terminal_write("0x");
                terminal_write(&buf[i]);
                terminal_write("\n");
            }
            terminal_write("pitch: ");
            {
                char buf[16];
                int n = fb.pitch, i = 15;
                buf[15] = 0;
                do
                {
                    buf[--i] = '0' + (n % 10);
                    n /= 10;
                } while (n && i > 0);
                terminal_write(&buf[i]);
                terminal_write("\n");
            }

            if (fb.bpp == 16 || fb.bpp == 24 || fb.bpp == 32)
                terminal_write("GUI working !!!");
            else
            {
                terminal_write("Framebuffer bpp not supported!\n");
                while (1)
                    __asm__("hlt");
            }
        }
        else
        {
            terminal_write("No framebuffer found!\n");
            while (1)
                __asm__("hlt");
        }
    }
    else
    {
        // Terminal mode
        terminal_clear();
        terminal_setcolor(0x1F); // Bright white on blue
        terminal_write("PulseOS Terminal\nType 'help' for options.\n");
        terminal_write("Loading Disk Drive...\n");
        fs_init();
        terminal_write("Loaded Disk Drive...\n");
        main_input_loop();
    }
}

// ============ Externs for apps =============

const char kbdus[128];
const char kbdus_shift[128];
volatile int shift_pressed;
uint8_t inb(uint16_t port);
void terminal_clear();
void terminal_write(const char *str);