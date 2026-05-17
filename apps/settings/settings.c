#include "settings.h"
#include "../../kernel/tasking/app.h"
#include "../../kernel/syscalls/syscall.h"
#include "../../gui/fb.h"
#include "../../gui/window.h"
#include "../../kernel/string.h"

extern int ui_theme;
extern int wallpaper_mode;
int current_tab = 0; // 0=Appearance, 1=System, 2=Audio

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
    wm_add_window(250, 200, 420, 320, "Settings", settings_draw);
    sys_create_task("settings", 1);
}

void settings_draw(void* self) {
    window_t* w = (window_t*)self;
    
    // Draw tabs
    draw_retro_3d_panel(w->x + 10, w->y + 30, 100, 25, current_tab == 0);
    fb_print("Appearance", w->x + 20, w->y + 38, 0x000000, 0xC0C0C0);
    
    draw_retro_3d_panel(w->x + 115, w->y + 30, 100, 25, current_tab == 1);
    fb_print("System Info", w->x + 125, w->y + 38, 0x000000, 0xC0C0C0);
    
    draw_retro_3d_panel(w->x + 220, w->y + 30, 100, 25, current_tab == 2);
    fb_print("Audio", w->x + 245, w->y + 38, 0x000000, 0xC0C0C0);
    
    // Panel area
    draw_retro_3d_panel(w->x + 10, w->y + 55, w->w - 20, w->h - 65, 1);
    
    int px = w->x + 20;
    int py = w->y + 70;
    
    if (current_tab == 0) {
        fb_print("Appearance Settings", px, py, 0x882200, 0xC0C0C0);
        draw_retro_3d_panel(px, py + 20, 180, 30, ui_theme == 1);
        fb_print("Retro Theme", px + 20, py + 31, 0x000000, 0xC0C0C0);
        draw_retro_3d_panel(px, py + 60, 180, 30, ui_theme == 0);
        fb_print("Modern Theme", px + 20, py + 71, 0x000000, 0xC0C0C0);
        
        fb_print("Wallpaper", px, py + 110, 0x882200, 0xC0C0C0);
        draw_retro_3d_panel(px, py + 130, 180, 30, wallpaper_mode == 1);
        fb_print("Solid Teal", px + 20, py + 141, 0x000000, 0xC0C0C0);
        draw_retro_3d_panel(px, py + 170, 180, 30, wallpaper_mode == 0);
        fb_print("Space Gradient", px + 20, py + 181, 0x000000, 0xC0C0C0);
    } else if (current_tab == 1) {
        fb_print("System Information", px, py, 0x224488, 0xC0C0C0);
        
        extern void cpuid_get_brand_string(char* buf);
        char cpu[64]; cpuid_get_brand_string(cpu);
        
        char line[128];
        xsprintf(line, "CPU: %s", cpu);
        fb_print(line, px, py + 30, 0x000000, 0xC0C0C0);
        
        mem_info_t mem;
        sys_get_mem_info(&mem);
        xsprintf(line, "Memory: %d MB Total", mem.total_memory/1024/1024);
        fb_print(line, px, py + 50, 0x000000, 0xC0C0C0);
        
        fb_print("OS: Onken OS Nightly Build", px, py + 70, 0x000000, 0xC0C0C0);
    } else if (current_tab == 2) {
        fb_print("Audio Settings", px, py, 0x882200, 0xC0C0C0);
        fb_print("Sound Driver: PC Speaker (Port 0x61)", px, py + 30, 0x000000, 0xC0C0C0);
        fb_print("Status: Active", px, py + 50, 0x00AA00, 0xC0C0C0);
        
        draw_retro_3d_panel(px, py + 90, 180, 30, 0);
        fb_print("Test Beep", px + 40, py + 101, 0x000000, 0xC0C0C0);
    }
}
