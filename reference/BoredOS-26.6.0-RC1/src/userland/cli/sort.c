// Copyright (c) 2023-2026 Christiaan (chris@boreddev.nl)
// This software is released under the GNU General Public License v3.0. See LICENSE file for details.
// This header needs to maintain in any file it is present in, as per the GPL license terms.

#include "syscall.h"
#include "libui.h"
#include "stdlib.h"

#define WIN_W 400
#define WIN_H 210
#define NUM_ELEM (WIN_W / 2)
#define BAR_W 2

static uint32_t *fb = NULL;
static int heights[NUM_ELEM];
static int moves = 0;

static unsigned long int next_rand = 1;

static int my_rand(void) {
    next_rand = next_rand * 1103515245 + 12345;
    return (unsigned int)(next_rand / 65536) % 32768;
}

static void my_srand(unsigned int seed) {
    next_rand = seed;
}

static void render_state(ui_window_t win) {
    // Clear fb
    for (int i = 0; i < WIN_W * WIN_H; i++) {
        fb[i] = 0xFF1E1E1E;
    }

    // Draw bars
    for (int i = 0; i < NUM_ELEM; i++) {
        int h = heights[i];
        if (h > WIN_H - 10) h = WIN_H - 10;
        int x = i * BAR_W;
        int y = WIN_H - h;
        
        // draw rect
        for (int yy = y; yy < WIN_H; yy++) {
            for (int xx = x; xx < x + BAR_W; xx++) {
                if (xx >= 0 && xx < WIN_W && yy >= 0 && yy < WIN_H) {
                    fb[yy * WIN_W + xx] = 0xFF4A90E2; // Blue
                }
            }
        }
    }

    // Draw Border for Box - bottom left
    int box_x = 10;
    int box_y = WIN_H - 40; // moved up by 10 pixels
    int box_w = 120;
    int box_h = 24;

    for (int yy = box_y; yy < box_y + box_h; yy++) {
        for (int xx = box_x; xx < box_x + box_w; xx++) {
            if (xx == box_x || xx == box_x + box_w - 1 || yy == box_y || yy == box_y + box_h - 1) {
                fb[yy * WIN_W + xx] = 0xFFFFFFFF;
            } else {
                fb[yy * WIN_W + xx] = 0xFF1E1E1E;
            }
        }
    }

    // Draw framebuffer to window
    ui_draw_image(win, 0, 0, WIN_W, WIN_H, fb);

    // Draw text inside the box
    char buf[32];
    strcpy(buf, "Moves: ");
    char num_buf[16];
    itoa(moves, num_buf);
    strcat(buf, num_buf);

    ui_draw_string(win, box_x + 8, box_y + 4, buf, 0xFFFFFFFF);
    ui_mark_dirty(win, 0, 0, WIN_W, WIN_H);
}

int main(void) {
    ui_window_t win = ui_window_create("sort", 100, 100, WIN_W, WIN_H);
    fb = (uint32_t*)malloc(WIN_W * WIN_H * sizeof(uint32_t));
    if (!fb) return 1;

    // Seed PRNG with system time (ticks)
    my_srand((unsigned int)sys_system(SYSTEM_CMD_GET_TICKS, 0, 0, 0, 0));

    // Initialize perfect slope
    int max_h = WIN_H - 40; // max height
    int min_h = 10;
    for (int i = 0; i < NUM_ELEM; i++) {
        heights[i] = min_h + (max_h - min_h) * i / (NUM_ELEM - 1);
    }

    // Shuffle
    for (int i = NUM_ELEM - 1; i > 0; i--) {
        int j = my_rand() % (i + 1);
        int t = heights[i];
        heights[i] = heights[j];
        heights[j] = t;
    }

    gui_event_t ev;

    // Cocktail shaker sort variables
    bool swapped = true;
    int start = 0;
    int end = NUM_ELEM - 1;
    bool done = false;

    // We render after each swap so the steps are extremely visual
    while (1) {
        if (ui_get_event(win, &ev)) {
            if (ev.type == GUI_EVENT_CLOSE) {
                break;
            }
        }

        if (!done) {
            swapped = false;
            
            // Forward pass
            for (int i = start; i < end; ++i) {
                if (heights[i] > heights[i + 1]) {
                    int t = heights[i];
                    heights[i] = heights[i+1];
                    heights[i+1] = t;
                    swapped = true;
                    moves++;
                    
                    render_state(win);
                    
                    // Allow UI events while sorting
                    if (ui_get_event(win, &ev)) {
                        if (ev.type == GUI_EVENT_CLOSE) {
                            goto exit_app;
                        }
                    }
                    sleep(2);
                }
            }

            if (!swapped) {
                done = true;
                continue;
            }

            swapped = false;
            end = end - 1;

            // Backward pass
            for (int i = end - 1; i >= start; --i) {
                if (heights[i] > heights[i + 1]) {
                    int t = heights[i];
                    heights[i] = heights[i+1];
                    heights[i+1] = t;
                    swapped = true;
                    moves++;
                    
                    render_state(win);
                    
                    // Allow UI events
                    if (ui_get_event(win, &ev)) {
                        if (ev.type == GUI_EVENT_CLOSE) {
                            goto exit_app;
                        }
                    }
                    sleep(2);
                }
            }
            start = start + 1;
        } else {
            // Sort is done, just render and idle
            render_state(win);
            sleep(200);
        }
    }

exit_app:
    free(fb);
    exit(0);
    return 0;
}
