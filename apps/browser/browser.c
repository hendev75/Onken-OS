#include "browser.h"
#include "../../kernel/tasking/app.h"
#include "../../kernel/syscalls/syscall.h"
#include "../../gui/fb.h"
#include "../../gui/window.h"
#include "../../kernel/string.h"

static app_entry_t browser_app = {
    .name = "browser",
    .title = "Onken Browser",
    .description = "Web Browser",
    .init = browser_init,
    .launch = browser_launch,
    .draw = browser_draw,
    .handle_key = 0
};

void browser_init(void) {
    app_register(&browser_app);
}

void browser_launch(const char* args) {
    (void)args;
    wm_add_window(150, 100, 800, 500, "Onken Browser", browser_draw);
    sys_create_task("browser", 1);
}

void browser_draw(void* self) {
    window_t* w = (window_t*)self;
    
    // Draw 3D sunken URL Address Bar container
    draw_retro_3d_panel(w->x + 10, w->y + 35, w->w - 20, 22, 1);
    fb_rect(w->x + 12, w->y + 37, w->w - 24, 18, 0xFFFFFF);
    fb_print("URL: https://onken-search.org", w->x + 16, w->y + 42, 0x000000, 0xFFFFFF);
    
    // Draw 3D sunken Web Viewport container
    draw_retro_3d_panel(w->x + 10, w->y + 65, w->w - 20, w->h - 75, 1);
    fb_rect(w->x + 12, w->y + 67, w->w - 24, w->h - 79, 0xFFFFFF); // Solid white webpage content area
    
    // Web Page Content (rendered on white background)
    fb_print("Onken Search Engine v1.0", w->x + (w->w - 24*8)/2, w->y + 85, 0x000000, 0xFFFFFF);
    
    // Search Submit Button (raised retro grey button)
    draw_retro_3d_panel(w->x + (w->w - 150)/2, w->y + 115, 150, 26, 0);
    fb_print("Search", w->x + (w->w - 6*8)/2, w->y + 124, 0x000000, 0xC0C0C0);
    
    // News Feed on white background
    fb_print("Featured News:", w->x + 24, w->y + 175, 0x882200, 0xFFFFFF);
    fb_print("- Onken OS revolutionizes OS development!", w->x + 24, w->y + 200, 0x000000, 0xFFFFFF);
    fb_print("- Diff-based rendering makes dragging butter-smooth.", w->x + 24, w->y + 220, 0x000000, 0xFFFFFF);
}
