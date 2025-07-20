#include <stdint.h>
#include <stddef.h>
#include "snake.h"

// VGA definitions must match those in kernel.c
#define VGA_TEXT_BUFFER ((volatile uint16_t*)0xB8000)
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#define SNAKE_WIDTH 40
#define SNAKE_HEIGHT 20
#define SNAKE_MAXLEN 100

typedef struct {
    int x, y;
} Point;

static Point snake[SNAKE_MAXLEN];
static size_t snake_len;
static Point food;
static int dx, dy;
static int game_over;

// Extern keyboard mappings from kernel.c
extern const char kbdus[128];
extern uint8_t inb(uint16_t port);

static void snake_draw() {
    // Clear screen
    for (size_t y = 0; y < SNAKE_HEIGHT; y++)
        for (size_t x = 0; x < SNAKE_WIDTH; x++)
            VGA_TEXT_BUFFER[y * VGA_WIDTH + x] = (0x2F << 8) | ' ';

    // Draw food
    VGA_TEXT_BUFFER[food.y * VGA_WIDTH + food.x] = (0x2F << 8) | '*';

    // Draw snake
    for (size_t i = 0; i < snake_len; i++)
        VGA_TEXT_BUFFER[snake[i].y * VGA_WIDTH + snake[i].x] = (0x2F << 8) | 'O';

    // Draw border
    for (size_t x = 0; x < SNAKE_WIDTH; x++) {
        VGA_TEXT_BUFFER[0 * VGA_WIDTH + x] = (0x2F << 8) | '#';
        VGA_TEXT_BUFFER[(SNAKE_HEIGHT-1) * VGA_WIDTH + x] = (0x2F << 8) | '#';
    }
    for (size_t y = 0; y < SNAKE_HEIGHT; y++) {
        VGA_TEXT_BUFFER[y * VGA_WIDTH + 0] = (0x2F << 8) | '#';
        VGA_TEXT_BUFFER[y * VGA_WIDTH + (SNAKE_WIDTH-1)] = (0x2F << 8) | '#';
    }
}

static void snake_init() {
    // Start horizontal, moving right, in the middle vertically
    int mid_y = SNAKE_HEIGHT / 2;
    int start_x = SNAKE_WIDTH / 4;
    snake_len = 3;
    for (size_t i = 0; i < snake_len; i++) {
        // Head at highest x, tail at lowest
        snake[i].x = start_x + snake_len - 1 - i;
        snake[i].y = mid_y;
    }
    dx = 1; dy = 0;
    // Place food away from snake
    food.x = SNAKE_WIDTH / 2;
    food.y = mid_y;
    game_over = 0;
}

static void snake_handle_input(char c) {
    if (c == 'w' && dy != 1) { dx = 0; dy = -1; }
    if (c == 's' && dy != -1) { dx = 0; dy = 1; }
    if (c == 'a' && dx != 1) { dx = -1; dy = 0; }
    if (c == 'd' && dx != -1) { dx = 1; dy = 0; }
}

static void snake_update() {
    // move snake
    Point new_head = {snake[0].x + dx, snake[0].y + dy};

    // check wall collision
    if (new_head.x <= 0 || new_head.x >= SNAKE_WIDTH-1 ||
        new_head.y <= 0 || new_head.y >= SNAKE_HEIGHT-1) {
        game_over = 1;
        return;
    }
    // check self collision (skip head)
    for (size_t i = 1; i < snake_len; i++)
        if (snake[i].x == new_head.x && snake[i].y == new_head.y) {
            game_over = 1;
            return;
        }

    // insert new head
    for (size_t i = snake_len; i > 0; i--)
        snake[i] = snake[i-1];
    snake[0] = new_head;

    if (new_head.x == food.x && new_head.y == food.y) {
        if (snake_len < SNAKE_MAXLEN)
            snake_len++;
        // Place new food (simple, not random)
        food.x = (food.x + 7) % (SNAKE_WIDTH-2) + 1;
        food.y = (food.y + 5) % (SNAKE_HEIGHT-2) + 1;
    } else {
        // remove tail
        if (snake_len > 0)
            snake_len--;
    }
}

void snake_game(void (*terminal_clear)(), void (*terminal_write)(const char*)) {
    snake_init();
    while (!game_over) {
        snake_draw();
        // Poll keyboard
        if (inb(0x64) & 0x01) {
            uint8_t sc = inb(0x60);
            if (!(sc & 0x80)) {
                char c = kbdus[sc];
                snake_handle_input(c);
            }
        }
        snake_update();

        // Delay
        for (volatile int i = 0; i < 2000000; i++);
    }
    terminal_clear();
    terminal_write("Game Over! Press any key to return.\n");
    // Wait for key
    while (!(inb(0x64) & 0x01));
}