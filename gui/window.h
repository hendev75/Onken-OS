#pragma once
#include <stdint.h>

typedef struct {
    uint32_t x, y, w, h;
    const char* title;
    uint32_t color;
    uint8_t active;
    uint8_t closed;
    void (*draw_content)(void* self);
} window_t;

#define MAX_WINDOWS 16

extern int ui_theme;

void wm_init();
void wm_add_window(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const char* title, void (*draw_fn)(void*));
void wm_draw_all();
void wm_handle_mouse(int mx, int my, int left_click);
window_t* wm_get_active();
