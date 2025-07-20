#include <stddef.h>
#include <stdint.h>
#include "../fs.h"

#define CALC_BUF_SIZE 128
static char calc_buf[CALC_BUF_SIZE];
static size_t calc_buf_len;

extern const char kbdus[128];
extern const char kbdus_shift[128];
extern volatile int shift_pressed;
extern uint8_t inb(uint16_t port);

static void calc_prompt(void (*terminal_write)(const char*)) {
    terminal_write("\nCalc> ");
    calc_buf_len = 0;
}

static int calc_parse_and_compute(const char* expr) {
    int a = 0, b = 0;
    char op = 0;
    int i = 0;
    while (expr[i] >= '0' && expr[i] <= '9') {
        a = a * 10 + (expr[i] - '0');
        i++;
    }
    if (expr[i] == '+' || expr[i] == '-' || expr[i] == '*' || expr[i] == '/') {
        op = expr[i];
        i++;
    }
    while (expr[i] >= '0' && expr[i] <= '9') {
        b = b * 10 + (expr[i] - '0');
        i++;
    }
    if (op == '+') return a + b;
    if (op == '-') return a - b;
    if (op == '*') return a * b;
    if (op == '/' && b != 0) return a / b;
    return 0;
}

static void calc_log(const char *expr, int result) {
    // Save the calculation to a persistent file "calc.log"
    char log_entry[64];
    int idx = 0;
    for (int i = 0; expr[i] && idx < 40; i++) log_entry[idx++] = expr[i];
    log_entry[idx++] = '=';
    int r = result;
    if (r < 0) { log_entry[idx++] = '-'; r = -r; }
    int digits[10], n = 0;
    do { digits[n++] = r % 10; r /= 10; } while (r && n < 10);
    for (int i = n-1; i >= 0; i--) log_entry[idx++] = '0' + digits[i];
    log_entry[idx++] = '\n';
    log_entry[idx] = 0;

    // Create log file if doesn't exist
    fs_create("calc.log", NULL);
    fs_write("calc.log",NULL, log_entry, idx);
}

void calc_app(void (*terminal_clear)(), void (*terminal_write)(const char*)) {
    terminal_clear();
    terminal_write("PulseOS Calc App\nType simple expressions like 5+3 and press Enter. ESC to exit.");
    calc_prompt(terminal_write);

    int running = 1;
    while (running) {
        if (inb(0x64) & 0x01) {
            uint8_t sc = inb(0x60);

            // Shift key logic
            if (sc == 0x2A || sc == 0x36) { shift_pressed = 1; continue; }
            if (sc == 0xAA || sc == 0xB6) { shift_pressed = 0; continue; }

            if (!(sc & 0x80)) { // key press
                char c = shift_pressed ? kbdus_shift[sc] : kbdus[sc];
                if (c == 27) { // ESC
                    running = 0;
                    continue;
                }
                if (c == '\b' && calc_buf_len > 0) {
                    calc_buf_len--;
                    terminal_write("\b \b");
                } else if (c == '\n') {
                    calc_buf[calc_buf_len] = '\0';
                    int result = calc_parse_and_compute(calc_buf);
                    char out[32];
                    int idx = 0;
                    int r = result;
                    if (r < 0) { out[idx++] = '-'; r = -r; }
                    int digits[10], n = 0;
                    do { digits[n++] = r % 10; r /= 10; } while (r);
                    for (int i = n-1; i >= 0; i--) out[idx++] = '0' + digits[i];
                    out[idx] = '\0';
                    terminal_write("\n = ");
                    terminal_write(out);
                    calc_log(calc_buf, result); // log each calculation to persistent file
                    calc_prompt(terminal_write);
                    calc_buf_len = 0;
                } else if (c && calc_buf_len < CALC_BUF_SIZE-1) {
                    calc_buf[calc_buf_len++] = c;
                    char out[2] = {c, '\0'};
                    terminal_write(out);
                }
            }
        }
        for (volatile int i = 0; i < 50000; i++); // Small delay
    }
    terminal_clear();
    terminal_write("Exiting Calc App.\n");
}