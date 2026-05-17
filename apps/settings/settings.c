#include "settings.h"
#include "../../kernel/tasking/app.h"
#include "../../kernel/syscalls/syscall.h"
#include "../../gui/fb.h"
#include "../../gui/window.h"
#include "../../kernel/string.h"

extern int ui_theme;
extern int wallpaper_mode;

static app_entry_t settings_app = {
    .name = "settings",
    .title = "Settings",
    .description = "Appearance Settings",
    .init = settings_init,
    .launch = settings_launch,
    .draw = settings_draw,
    .handle_key = 0
};

void settings_init(void) {
    app_register(&settings_app);
}

void settings_launch(const char* args) {
    (void)args;
    wm_add_window(250, 200, 400, 300, "Settings", settings_draw);
    sys_create_task("settings", 1);
}

void settings_draw(void* self) {
    window_t* w = (window_t*)self;
    fb_print("Appearance Settings", w->x + 20, w->y + 40, 0x882200, 0xC0C0C0);
    
    // Retro Theme Button (sunken when active)
    draw_retro_3d_panel(w->x + 20, w->y + 70, 200, 30, ui_theme == 1);
    fb_print("Retro Theme", w->x + 30, w->y + 81, 0x000000, 0xC0C0C0);
    
    // Modern Theme Button (sunken when active)
    draw_retro_3d_panel(w->x + 20, w->y + 110, 200, 30, ui_theme == 0);
    fb_print("Modern Theme", w->x + 30, w->y + 121, 0x000000, 0xC0C0C0);
 
    fb_print("Wallpaper Settings", w->x + 20, w->y + 160, 0x882200, 0xC0C0C0);
 
    // Solid Teal Button (sunken when active)
    draw_retro_3d_panel(w->x + 20, w->y + 190, 200, 30, wallpaper_mode == 1);
    fb_print("Solid Teal", w->x + 30, w->y + 201, 0x000000, 0xC0C0C0);
 
    // BoredOS Space Button (sunken when active)
    draw_retro_3d_panel(w->x + 20, w->y + 230, 200, 30, wallpaper_mode == 0);
    fb_print("BoredOS Space", w->x + 30, w->y + 241, 0x000000, 0xC0C0C0);
}
