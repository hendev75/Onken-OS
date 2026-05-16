#include "window.h"
#include "fb.h"
#include "../kernel/string.h"

static window_t windows[MAX_WINDOWS];
static uint32_t window_count = 0;
static window_t* active_window = 0;
static int dragging_window_idx = -1;
static int drag_off_x = 0, drag_off_y = 0;

theme_t current_theme = {
    .border_light = 0xFFFFFF,
    .border_dark = 0x555555,
    .border_mid = 0xC0C0C0,
    .titlebar_active = 0x224488,         // Classic Active Royal Blue
    .titlebar_active_grad = 0x6688DD,    // Light Blue Gradient
    .titlebar_inactive = 0x555555,       // Inactive Dark Grey
    .titlebar_inactive_grad = 0x888888,  // Inactive Light Grey
    .bg_desktop = 0x008080,              // Retro Workbench Teal
    .text_active = 0xFFFFFF,
    .text_inactive = 0xCCCCCC
};

void wm_init() {
    window_count = 0;
    active_window = 0;
}

void draw_retro_3d_panel(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t sunken) {
    uint32_t l = current_theme.border_light;
    uint32_t d = current_theme.border_dark;
    uint32_t m = current_theme.border_mid;
    
    if (sunken) {
        l = current_theme.border_dark;
        d = current_theme.border_light;
    }
    
    // Fill surface
    fb_rect(x + 2, y + 2, w - 4, h - 4, m);
    
    // Outer Bevel (1px)
    fb_rect(x, y, w, 1, l);            // Top outer
    fb_rect(x, y, 1, h, l);            // Left outer
    fb_rect(x + w - 1, y, 1, h, d);     // Right outer
    fb_rect(x, y + h - 1, w, 1, d);     // Bottom outer
    
    // Inner Bevel (1px)
    uint32_t il = sunken ? 0x222222 : 0xDFDFDF;
    uint32_t id = sunken ? 0xDFDFDF : 0x808080;
    fb_rect(x + 1, y + 1, w - 2, 1, il); // Top inner
    fb_rect(x + 1, y + 1, 1, h - 2, il); // Left inner
    fb_rect(x + w - 2, y + 1, 1, h - 2, id); // Right inner
    fb_rect(x + 1, y + h - 2, w - 2, 1, id); // Bottom inner
}

void wm_add_window(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const char* title, void (*draw_fn)(void*)) {
    if (window_count >= MAX_WINDOWS) return;
    
    windows[window_count] = (window_t){
        .x = x, .y = y, .w = w, .h = h,
        .title = title,
        .color = 0xC0C0C0,
        .active = 1,
        .closed = 0,
        .minimized = 0,
        .draw_content = draw_fn
    };
    
    if (active_window) {
        active_window->active = 0;
    }
    
    active_window = &windows[window_count];
    window_count++;
}

static void draw_window_frame(window_t* w) {
    uint8_t is_active = (w == active_window);
    uint32_t t_c1 = is_active ? current_theme.titlebar_active : current_theme.titlebar_inactive;
    uint32_t t_c2 = is_active ? current_theme.titlebar_active_grad : current_theme.titlebar_inactive_grad;
    uint32_t text_col = is_active ? current_theme.text_active : current_theme.text_inactive;
    
    // Draw 3D outer frame (Height is 24px if minimized/shaded, otherwise full h)
    uint32_t border_h = w->minimized ? 26 : w->h;
    draw_retro_3d_panel(w->x, w->y, w->w, border_h, 0);
    
    // Titlebar gradient area (raised retro look)
    fb_gradient(w->x + 4, w->y + 4, w->w - 8, 18, t_c1, t_c2);
    fb_print(w->title, w->x + 26, w->y + 9, text_col, t_c1);
    
    // 1. Close Gadget (Left corner)
    // Draw small 3D sunken button
    draw_retro_3d_panel(w->x + 6, w->y + 6, 14, 14, 1);
    fb_print("x", w->x + 10, w->y + 9, 0x000000, current_theme.border_mid);
    
    // 2. Depth Gadget (Right corner)
    // Draw overlapping square icons
    draw_retro_3d_panel(w->x + w->w - 20, w->y + 6, 14, 14, 0);
    fb_rect(w->x + w->w - 17, w->y + 9, 6, 6, 0x555555);
    fb_rect(w->x + w->w - 14, w->y + 12, 6, 6, 0xFFFFFF);
    
    // 3. Minimize Gadget (Window shading)
    draw_retro_3d_panel(w->x + w->w - 38, w->y + 6, 14, 14, 0);
    fb_print(w->minimized ? "+" : "-", w->x + w->w - 34, w->y + 9, 0x000000, current_theme.border_mid);
    
    if (!w->minimized) {
        // Inner Content Sunken Panel
        draw_retro_3d_panel(w->x + 4, w->y + 24, w->w - 8, w->h - 28, 1);
    }
}

void wm_draw_all() {
    // 1. Draw inactive windows first (front-to-back sorting base)
    for (uint32_t i = 0; i < window_count; i++) {
        window_t* w = &windows[i];
        if (w->closed || w == active_window) continue;
        
        draw_window_frame(w);
        if (!w->minimized && w->draw_content) {
            w->draw_content(w);
        }
    }
    
    // 2. Draw active window last (on top of all others)
    if (active_window && !active_window->closed) {
        draw_window_frame(active_window);
        if (!active_window->minimized && active_window->draw_content) {
            active_window->draw_content(active_window);
        }
    }
}

void wm_cycle_depth() {
    if (!active_window || window_count <= 1) return;
    
    // Find active window index
    int active_idx = -1;
    for (uint32_t i = 0; i < window_count; i++) {
        if (&windows[i] == active_window) {
            active_idx = i;
            break;
        }
    }
    
    if (active_idx == -1) return;
    
    // Shift current active window to the very bottom of the window array (z-index = 0)
    window_t temp = windows[active_idx];
    for (int i = active_idx; i > 0; i--) {
        windows[i] = windows[i - 1];
    }
    windows[0] = temp;
    
    // Make the topmost non-closed window active
    active_window->active = 0;
    active_window = 0;
    for (int i = (int)window_count - 1; i >= 0; i--) {
        if (!windows[i].closed) {
            active_window = &windows[i];
            active_window->active = 1;
            break;
        }
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
    
    // Front-to-back click detection
    // 1. Check active window first
    if (active_window && !active_window->closed) {
        window_t* w = active_window;
        if (mx >= (int)w->x && mx <= (int)(w->x + w->w) &&
            my >= (int)w->y && my <= (int)(w->y + 24)) {
            
            // Check Close Box
            if (mx >= (int)w->x + 6 && mx <= (int)w->x + 20 &&
                my >= (int)w->y + 6 && my <= (int)w->y + 20) {
                w->closed = 1;
                active_window = 0;
                // Switch to next active window
                for (int i = (int)window_count - 1; i >= 0; i--) {
                    if (!windows[i].closed) {
                        active_window = &windows[i];
                        active_window->active = 1;
                        break;
                    }
                }
                return;
            }
            
            // Check Depth Gadget (toggles back)
            if (mx >= (int)w->x + w->w - 20 && mx <= (int)w->x + w->w - 6 &&
                my >= (int)w->y + 6 && my <= (int)w->y + 20) {
                wm_cycle_depth();
                return;
            }
            
            // Check Minimize Box (shaded)
            if (mx >= (int)w->x + w->w - 38 && mx <= (int)w->x + w->w - 24 &&
                my >= (int)w->y + 6 && my <= (int)w->y + 20) {
                w->minimized = !w->minimized;
                return;
            }
            
            // Drag titlebar
            dragging_window_idx = active_window - windows;
            drag_off_x = mx - w->x;
            drag_off_y = my - w->y;
            return;
        }
    }
    
    // 2. Check other windows
    for (int i = (int)window_count - 1; i >= 0; i--) {
        window_t* w = &windows[i];
        if (w->closed || w == active_window) continue;
        
        uint32_t click_h = w->minimized ? 24 : w->h;
        if (mx >= (int)w->x && mx <= (int)(w->x + w->w) &&
            my >= (int)w->y && my <= (int)(w->y + click_h)) {
            
            // Focus clicked window
            if (active_window) active_window->active = 0;
            w->active = 1;
            active_window = w;
            
            // Allow immediate dragging if clicked on titlebar
            if (my <= (int)w->y + 24) {
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