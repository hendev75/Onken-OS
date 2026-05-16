#pragma once
#include <stdint.h>

typedef struct {
    uint32_t border_light;
    uint32_t border_dark;
    uint32_t border_mid;
    uint32_t titlebar_active;
    uint32_t titlebar_active_grad;
    uint32_t titlebar_inactive;
    uint32_t titlebar_inactive_grad;
    uint32_t bg_desktop;
    uint32_t text_active;
    uint32_t text_inactive;
} theme_t;

typedef struct {
    uint32_t x, y, w, h;
    const char* title;
    uint32_t color;
    uint8_t active;
    uint8_t closed;
    uint8_t minimized;
    void (*draw_content)(void* self);
} window_t;

#define MAX_WINDOWS 16

extern int ui_theme;
extern theme_t current_theme;

void wm_init();
void wm_add_window(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const char* title, void (*draw_fn)(void*));
void wm_draw_all();
void wm_handle_mouse(int mx, int my, int left_click);
window_t* wm_get_active();
void draw_retro_3d_panel(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t sunken);
void wm_cycle_depth();
