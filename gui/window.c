#include "window.h"
#include "fb.h"
#include "../kernel/string.h"

static window_t windows[MAX_WINDOWS];
static uint32_t window_count = 0;
static window_t* active_window = 0;
static int dragging_window_idx = -1;
static int drag_off_x = 0, drag_off_y = 0;

void wm_init() {
    window_count = 0;
    active_window = 0;
}

void wm_add_window(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const char* title, void (*draw_fn)(void*)) {
    if (window_count >= MAX_WINDOWS) return;
    
    windows[window_count] = (window_t){
        .x = x, .y = y, .w = w, .h = h,
        .title = title,
        .color = 0x2C3E50,
        .active = 1,
        .closed = 0,
        .draw_content = draw_fn
    };
    
    // Set previous active to inactive
    if (active_window) {
        active_window->active = 0;
    }
    
    active_window = &windows[window_count];
    window_count++;
}

void wm_draw_all() {
    for (uint32_t i = 0; i < window_count; i++) {
        window_t* w = &windows[i];
        if (w->closed) continue;
        
        // Z-order: active window drawn on top of others
        if (w == active_window) continue;
        
        if (ui_theme == 1) {
            // Retro inactive border
            fb_rect(w->x, w->y, w->w, w->h, 0xC0C0C0);
            fb_rect(w->x + 2, w->y + 2, w->w - 4, 20, 0x808080); // Titlebar
            fb_print(w->title, w->x + 4, w->y + 6, 0xC0C0C0, 0x808080);
            // Content
            fb_rect(w->x + 2, w->y + 22, w->w - 4, w->h - 24, 0x000000);
            // Close button
            fb_rect(w->x + w->w - 20, w->y + 4, 16, 16, 0xC0C0C0);
            fb_print("X", w->x + w->w - 16, w->y + 8, 0x000000, 0xC0C0C0);
        } else {
            // Inactive window border (Modern)
            fb_rect(w->x, w->y, w->w, w->h, 0x7F8C8D);
            fb_rect(w->x + 2, w->y + 2, w->w - 4, w->h - 4, 0x111111);
            fb_rect(w->x + 2, w->y + 2, w->w - 4, 28, 0x34495E); // titlebar
            fb_print(w->title, w->x + 12, w->y + 8, 0xBDC3C7, 0x34495E);
            
            // Close button
            fb_rect(w->x + w->w - 25, w->y + 5, 20, 20, 0x95A5A6);
            fb_print("X", w->x + w->w - 19, w->y + 8, 0x111111, 0x95A5A6);
        }
        
        if (w->draw_content) w->draw_content(w);
    }
    
    // Draw active window on top
    if (active_window && !active_window->closed) {
        window_t* w = active_window;
        if (ui_theme == 1) {
            // Retro active border
            fb_rect(w->x, w->y, w->w, w->h, 0xFFFFFF); // Outer light
            fb_rect(w->x + 1, w->y + 1, w->w - 1, w->h - 1, 0x808080); // Outer dark
            fb_rect(w->x + 1, w->y + 1, w->w - 2, w->h - 2, 0xDFDFDF); // Outer light
            fb_rect(w->x + 2, w->y + 2, w->w - 3, w->h - 3, 0x000000); // Inner dark
            fb_rect(w->x + 2, w->y + 2, w->w - 4, w->h - 4, 0xC0C0C0); // Main surface
            
            // Title Bar
            fb_rect(w->x + 4, w->y + 4, w->w - 8, 20, 0x000080); // Dark blue title
            fb_print(w->title, w->x + 6, w->y + 10, 0xFFFFFF, 0x000080);
            
            // Content
            fb_rect(w->x + 4, w->y + 26, w->w - 8, w->h - 30, 0x000000);
            
            // Close button
            fb_rect(w->x + w->w - 22, w->y + 6, 16, 16, 0xDFDFDF);
            fb_rect(w->x + w->w - 21, w->y + 7, 14, 14, 0xC0C0C0);
            fb_print("X", w->x + w->w - 18, w->y + 10, 0x000000, 0xC0C0C0);
        } else {
            // Drop shadow
            fb_rect(w->x + 6, w->y + 6, w->w, w->h, 0x000000); 

            // Premium active window border
            fb_rect(w->x, w->y, w->w, w->h, 0x2C3E50); 
            fb_rect(w->x + 1, w->y + 1, w->w - 2, w->h - 2, 0x34495E);
            fb_rect(w->x + 2, w->y + 30, w->w - 4, w->h - 32, 0x111111);

            // Active Title Bar
            fb_rect(w->x + 2, w->y + 2, w->w - 4, 28, 0x212121);
            fb_print(w->title, w->x + 12, w->y + 8, 0xFFFFFF, 0x212121);
            
            // Close button
            fb_rect(w->x + w->w - 25, w->y + 5, 20, 20, 0xC0392B);
            fb_print("X", w->x + w->w - 19, w->y + 8, 0xFFFFFF, 0xC0392B);
        }
        
        if (w->draw_content) w->draw_content(w);
    }
}

void wm_handle_mouse(int mx, int my, int left_click) {
    if (!left_click) {
        dragging_window_idx = -1;
        return;
    }
    
    if (dragging_window_idx != -1) {
        window_t* w = &windows[dragging_window_idx];
        w->x = mx - drag_off_x;
        w->y = my - drag_off_y;
        return;
    }
    
    // Check if clicked inside windows (front-to-back checking)
    // First, check active window
    if (active_window && !active_window->closed) {
        window_t* w = active_window;
        // Check Close button
        if (mx >= (int)w->x + (int)w->w - 25 && mx <= (int)w->x + (int)w->w - 5 &&
            my >= (int)w->y + 5 && my <= (int)w->y + 25) {
            w->closed = 1;
            active_window = 0;
            return;
        }
        
        // Check Title Bar dragging
        if (mx >= (int)w->x && mx <= (int)(w->x + w->w) &&
            my >= (int)w->y && my <= (int)(w->y + 30)) {
            dragging_window_idx = active_window - windows; // get index
            drag_off_x = mx - w->x;
            drag_off_y = my - w->y;
            return;
        }
    }
    
    // Check other windows
    for (int i = (int)window_count - 1; i >= 0; i--) {
        window_t* w = &windows[i];
        if (w->closed) continue;
        if (w == active_window) continue;
        
        if (mx >= (int)w->x && mx <= (int)(w->x + w->w) &&
            my >= (int)w->y && my <= (int)(w->y + w->h)) {
            
            // Bring to front
            if (active_window) active_window->active = 0;
            w->active = 1;
            active_window = w;
            
            // Check dragging for this window
            if (my <= (int)w->y + 30) {
                dragging_window_idx = i;
                drag_off_x = mx - w->x;
                drag_off_y = my - w->y;
            }
            return;
        }
    }
}

window_t* wm_get_active() {
    return active_window;
}