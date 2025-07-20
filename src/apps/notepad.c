#include "../fs.h"
#include <stddef.h>
#include <stdint.h>

extern const char kbdus[128];
extern volatile int shift_pressed;
extern uint8_t inb(uint16_t port);

#define NOTEPAD_BUF_SIZE 4096

void notepad_app(void (*terminal_clear)(), void (*terminal_write)(const char*),
                 int read_mode, const char* filename) {
    char buf[NOTEPAD_BUF_SIZE];
    size_t len = 0;
    const char *fname = (filename && filename[0]) ? filename : "notepad.txt";

    terminal_clear();

    if (read_mode) {
        // Read file from persistent storage
        int n = fs_read(fname, buf, NOTEPAD_BUF_SIZE-1);
        if (n > 0) {
            buf[n] = 0;
            terminal_write("Notepad - Read mode\n");
            terminal_write(buf);
        } else {
            terminal_write("Notepad - file is empty or cannot be read.\n");
        }
    } else {
        terminal_write("Notepad - Write mode\nType text, press ESC to save & exit.\n");
        len = 0;
        int running = 1;
        while (running && len < NOTEPAD_BUF_SIZE-1) {
            if (inb(0x64) & 0x01) {
                uint8_t sc = inb(0x60);
                char c = kbdus[sc];
                if (!(sc & 0x80)) {
                    if (c == 27) { // ESC: save & exit
                        buf[len] = 0;
                        fs_create(fname, NULL); // Ensure file exists before writing
                        fs_write(fname,NULL, buf, len);
                        running = 0;
                        continue;
                    }
                    if (c == '\b' && len > 0) {
                        len--;
                        terminal_write("\b \b");
                    } else if (c && len < NOTEPAD_BUF_SIZE-1) {
                        buf[len++] = c;
                        char out[2] = {c, '\0'};
                        terminal_write(out);
                    }
                }
            }
            for (volatile int i = 0; i < 50000; i++);
        }
        terminal_write("\nNotepad - Saved!\n");
    }
}