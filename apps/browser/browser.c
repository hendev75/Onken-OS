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
    fb_rect(w->x + 10, w->y + 35, w->w - 20, 20, 0x222222);
    fb_print("URL: https://onken-search.org", w->x + 15, w->y + 40, 0x00FF00, 0x222222);
    
    fb_print("Onken Search Engine v1.0", w->x + (w->w - 24*8)/2, w->y + 80, 0xFFFFFF, 0x111111);
    fb_rect(w->x + (w->w - 200)/2, w->y + 120, 200, 30, 0x333333);
    fb_print("Search", w->x + (w->w - 6*8)/2, w->y + 130, 0xFFFFFF, 0x333333);
    
    fb_print("Featured News:", w->x + 20, w->y + 180, 0xF5A623, 0x111111);
    fb_print("- Onken OS revolutionizes OS development!", w->x + 20, w->y + 205, 0xBDC3C7, 0x111111);
    fb_print("- Diff-based rendering makes dragging butter-smooth.", w->x + 20, w->y + 225, 0xBDC3C7, 0x111111);
}
