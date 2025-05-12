#include "ripes_system.h"
#include <stdio.h>

#define SWITCH_0 (0x01)
#define DIRECTION_HORIZONTAL 1
#define DIRECTION_VERTICAL 2
#define INITIAL_POSITION 2 * LED_MATRIX_0_WIDTH + 2
#define WALL_COLOR 0xFFFF00
#define BODY_COLOR 0xFF0000
#define FOOD_COLOR 0x00FF00
#define LED_MATRIX_0_BASE ((volatile unsigned int *)0xf0000000)

volatile unsigned int *head = LED_MATRIX_0_BASE;
volatile unsigned int *matrix_cleaner = LED_MATRIX_0_BASE;
volatile unsigned int *pad_up = D_PAD_0_UP;
volatile unsigned int *pad_down = D_PAD_0_DOWN;
volatile unsigned int *pad_left = D_PAD_0_LEFT;
volatile unsigned int *pad_right = D_PAD_0_RIGHT;
volatile unsigned int *food = LED_MATRIX_0_BASE;
volatile unsigned int *switches = SWITCHES_0_BASE;

unsigned int random_seed = 0;
volatile unsigned int *snake_segments[LED_MATRIX_0_WIDTH * LED_MATRIX_0_HEIGHT];
unsigned int length = 2;
unsigned int food_count = 0;

void drawBorders();
void resetMatrix();
void disableLEDs();
void spawnFood();
int random();
void setSeed(unsigned int root);

void main() {
    unsigned int switch_state = 1;
    unsigned int movement_state = DIRECTION_HORIZONTAL;
    unsigned int movement_offset = 1;

    unsigned int *tail_down = 0;
    unsigned int *tail_up = 0;
    unsigned int *head_top = 0;

    unsigned int *clear_segment1 = 0;
    unsigned int *clear_segment2 = 0;
    unsigned int *clear_segment3 = 0;

    int iteration = 0;

    head = INITIAL_POSITION;
    drawBorders();

    while (1) {
        if (food_count < 1) {
            disableLEDs();
        }

        tail_down = head - 1;
        tail_up = tail_down - LED_MATRIX_0_WIDTH;
        head_top = head - LED_MATRIX_0_WIDTH;

        if (*pad_up == 1 && movement_state == DIRECTION_HORIZONTAL) {
            movement_offset = -LED_MATRIX_0_WIDTH;
            movement_state = DIRECTION_VERTICAL;
        }
        if (*pad_down == 1 && movement_state == DIRECTION_HORIZONTAL) {
            movement_offset = LED_MATRIX_0_WIDTH;
            movement_state = DIRECTION_VERTICAL;
        }
        if (*pad_left == 1 && movement_state == DIRECTION_VERTICAL) {
            movement_offset = -1;
            movement_state = DIRECTION_HORIZONTAL;
        }
        if (*pad_right == 1 && movement_state == DIRECTION_VERTICAL) {
            movement_offset = 1;
            movement_state = DIRECTION_HORIZONTAL;
        }

        head += 2 * movement_offset;
        tail_down += 2 * movement_offset;
        tail_up += 2 * movement_offset;
        head_top += 2 * movement_offset;

        for (int i = length; i > 0; i--) {
            snake_segments[i] = snake_segments[i - 1];
        }
        snake_segments[0] = head;

        clear_segment1 = snake_segments[length] - 1;
        clear_segment2 = clear_segment1 - LED_MATRIX_0_WIDTH;
        clear_segment3 = snake_segments[length] - LED_MATRIX_0_WIDTH;

        *snake_segments[length] = 0x000000;
        *clear_segment1 = 0x000000;
        *clear_segment2 = 0x000000;
        *clear_segment3 = 0x000000;

        if (*head == FOOD_COLOR) {
            spawnFood();
            length++;
        }

        if (*head != 0x000000 && *head != FOOD_COLOR) {
            while (switch_state == 0) {
                switch_state = *switches & SWITCH_0;
                for (int i = 0; i < 3000; i++);
            }

            switch_state = 0;
            resetMatrix();
            spawnFood();

            matrix_cleaner = LED_MATRIX_0_BASE;
            head = LED_MATRIX_0_BASE;
            head += INITIAL_POSITION;
            tail_down = head - 1;
            tail_up = tail_down - LED_MATRIX_0_WIDTH;
            head_top = head - LED_MATRIX_0_WIDTH;
            snake_segments[0] = 0;
            clear_segment1 = 0;
            clear_segment2 = 0;
            clear_segment3 = 0;
            length = 2;
            food_count = 0;

            movement_offset = 1;
            movement_state = DIRECTION_HORIZONTAL;
        }

        *head = BODY_COLOR;
        *tail_down = BODY_COLOR;
        *tail_up = BODY_COLOR;
        *head_top = BODY_COLOR;

        for (int i = 0; i < 3000; i++);
    }
}

void drawBorders() {
    unsigned int *border_ptr = LED_MATRIX_0_BASE;
    unsigned int *border_aux = 0;

    for (int i = 0; i < 35; i++) {
        *border_ptr = WALL_COLOR;
        border_ptr++;
    }

    for (int i = 0; i < 25; i++) {
        *border_ptr = WALL_COLOR;
        border_aux = border_ptr - 1;
        *border_aux = WALL_COLOR;
        border_ptr += LED_MATRIX_0_WIDTH;
    }

    for (int i = 0; i < 35; i++) {
        *border_ptr = WALL_COLOR;
        border_aux = border_ptr - LED_MATRIX_0_WIDTH;
        *border_aux = WALL_COLOR;
        border_ptr--;
    }

    for (int i = 0; i < 25; i++) {
        *border_ptr = WALL_COLOR;
        border_ptr -= LED_MATRIX_0_WIDTH;
    }
}

void resetMatrix() {
    matrix_cleaner = LED_MATRIX_0_BASE;
    for (int i = 0; i < LED_MATRIX_0_WIDTH * LED_MATRIX_0_HEIGHT; i++) {
        if (*matrix_cleaner != WALL_COLOR) {
            *matrix_cleaner = 0x000000;
        }
        matrix_cleaner += 1;
    }
}

void disableLEDs() {
    volatile unsigned int *base_led = LED_MATRIX_0_BASE;
    volatile unsigned int *led1 = base_led + 1 * LED_MATRIX_0_WIDTH + 1;
    volatile unsigned int *led2 = led1 + 1;
    volatile unsigned int *led3 = led1 + LED_MATRIX_0_WIDTH;
    volatile unsigned int *led4 = led2 + LED_MATRIX_0_WIDTH;

    *led1 = 0x000000;
    *led2 = 0x000000;
    *led3 = 0x000000;
    *led4 = 0x000000;
}

int random() {
    unsigned int head_position = (unsigned int)head;
    random_seed = random_seed * 435324123 + head_position;
    return (unsigned int)(random_seed / 65536) % 32768;
}

void setSeed(unsigned int root) {
    random_seed = root;
}

void spawnFood() {
    setSeed(random_seed);

    int num_positions_x = (LED_MATRIX_0_WIDTH - 4) / 2;
    int num_positions_y = (LED_MATRIX_0_HEIGHT - 4) / 2;

    int coord_x = 0;
    int coord_y = 0;

    int apple;
    int apple_pos[4];

    do {
        coord_x = (random() % num_positions_x) * 2 + 2 - 1;
        coord_y = (random() % num_positions_y) * 2 + 2 - 1;

        apple = coord_y * LED_MATRIX_0_WIDTH + coord_x;

        apple_pos[0] = apple;
        apple_pos[1] = apple + 1;
        apple_pos[2] = apple + LED_MATRIX_0_WIDTH;
        apple_pos[3] = apple + LED_MATRIX_0_WIDTH + 1;

    } while (LED_MATRIX_0_BASE[apple_pos[0]] != 0x000000 ||
             LED_MATRIX_0_BASE[apple_pos[1]] != 0x000000 ||
             LED_MATRIX_0_BASE[apple_pos[2]] != 0x000000 ||
             LED_MATRIX_0_BASE[apple_pos[3]] != 0x000000);

    LED_MATRIX_0_BASE[apple_pos[0]] = FOOD_COLOR;
    LED_MATRIX_0_BASE[apple_pos[1]] = FOOD_COLOR;
    LED_MATRIX_0_BASE[apple_pos[2]] = FOOD_COLOR;
    LED_MATRIX_0_BASE[apple_pos[3]] = FOOD_COLOR;

    random_seed++;
    food_count++;
}