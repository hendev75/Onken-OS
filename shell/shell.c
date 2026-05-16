#include "../kernel/kernel.h"
#include "../gui/fb.h"
#include "../drivers/ps2.h"
#include "../kernel/string.h"
#include "../kernel/fs.h"
#include "../gui/window.h"

// Dynamic App and Subsystem Headers
#include "../kernel/tasking/app.h"
#include "../kernel/syscalls/syscall.h"
#include "../kernel/scheduler/scheduler.h"
#include "../kernel/time/pit.h"

#include "../apps/htop/htop.h"
#include "../apps/yano/yano.h"
#include "../apps/sysinfo/sysinfo.h"
#include "../apps/browser/browser.h"
#include "../apps/files/files.h"
#include "../apps/settings/settings.h"
#include "../apps/terminal/terminal.h"

int ui_theme = 1; // 1 = Retro, 0 = Modern
int wallpaper_mode = 1; // 0 = Space, 1 = Teal

// Icons on desktop
typedef struct {
    int x, y, w, h;
    const char* label;
    const char* app_name;
    uint32_t color;
} icon_t;

static icon_t icons[] = {
    {40, 40, 64, 64, "Browser", "browser", 0x4A90E2},
    {40, 140, 64, 64, "Terminal", "terminal", 0x50E3C2},
    {40, 240, 64, 64, "Files", "files", 0xF5A623}
};

// Start Menu state
static int start_menu_visible = 0;
static int menu_visible = 0;
static int menu_x = 0, menu_y = 0;

static uint32_t last_click_time = 0;
static int last_click_x = -1;

static int task_switcher_visible = 0;

static void draw_icon(icon_t* icon) {
    if (ui_theme == 1) {
        fb_rect(icon->x, icon->y, icon->w, icon->h, 0xC0C0C0);
        fb_rect(icon->x+2, icon->y+2, icon->w-4, icon->h-4, icon->color);
        fb_print(icon->label, icon->x + (icon->w - strlen(icon->label)*8)/2, icon->y + icon->h + 4, 0xFFFFFF, 0x008080);
    } else {
        fb_rect(icon->x, icon->y, icon->w, icon->h, icon->color);
        fb_rect(icon->x + 4, icon->y + 4, icon->w - 8, icon->h - 8, 0xFFFFFF);
        fb_rect(icon->x + 8, icon->y + 8, icon->w - 16, icon->h - 16, icon->color);
        fb_print(icon->label, icon->x + (icon->w - strlen(icon->label)*8)/2, icon->y + icon->h + 8, 0xFFFFFF, 0x000000);
    }
}

static void draw_context_menu(int x, int y) {
    if (ui_theme == 1) {
        fb_rect(x, y, 150, 80, 0xFFFFFF);
        fb_rect(x+1, y+1, 148, 78, 0xC0C0C0);
    } else {
        fb_rect(x, y, 150, 80, 0x222222);
        fb_rect(x+1, y+1, 148, 78, 0x444444);
    }
    fb_print("Refresh", x + 10, y + 10, 0xFFFFFF, ui_theme == 1 ? 0xC0C0C0 : 0x444444);
    fb_print("Wallpaper", x + 10, y + 35, 0xFFFFFF, ui_theme == 1 ? 0xC0C0C0 : 0x444444);
    fb_print("Settings", x + 10, y + 60, 0xFFFFFF, ui_theme == 1 ? 0xC0C0C0 : 0x444444);
}

extern int32_t mouse_x;
extern int32_t mouse_y;

static void draw_start_menu() {
    uint32_t bg = ui_theme == 1 ? 0xC0C0C0 : 0x34495E;
    uint32_t border = ui_theme == 1 ? 0xFFFFFF : 0x222222;
    uint32_t text_col = 0xFFFFFF;
    uint32_t hover_bg = ui_theme == 1 ? 0x000080 : 0x4A90E2; // Navy blue on hover
    
    fb_rect(10, fb_height - 350, 200, 300, border);
    fb_rect(11, fb_height - 349, 198, 298, bg);
    
    fb_print("Onken OS PRO", 20, fb_height - 330, ui_theme == 1 ? 0x000080 : 0xF5A623, bg);
    fb_rect(20, fb_height - 310, 180, 2, 0x7F8C8D);
    
    // Programs (Browser)
    if (mouse_x >= 10 && mouse_x <= 210 && mouse_y >= (int32_t)fb_height - 290 && mouse_y <= (int32_t)fb_height - 270) {
        fb_rect(15, fb_height - 295, 190, 25, hover_bg);
        fb_print("Programs", 25, fb_height - 290, text_col, hover_bg);
    } else {
        fb_print("Programs", 25, fb_height - 290, text_col, bg);
    }

    // System Info
    if (mouse_x >= 10 && mouse_x <= 210 && mouse_y >= (int32_t)fb_height - 250 && mouse_y <= (int32_t)fb_height - 230) {
        fb_rect(15, fb_height - 255, 190, 25, hover_bg);
        fb_print("System Info", 25, fb_height - 250, text_col, hover_bg);
    } else {
        fb_print("System Info", 25, fb_height - 250, text_col, bg);
    }

    // Filesystem
    if (mouse_x >= 10 && mouse_x <= 210 && mouse_y >= (int32_t)fb_height - 210 && mouse_y <= (int32_t)fb_height - 190) {
        fb_rect(15, fb_height - 215, 190, 25, hover_bg);
        fb_print("Filesystem", 25, fb_height - 210, text_col, hover_bg);
    } else {
        fb_print("Filesystem", 25, fb_height - 210, text_col, bg);
    }

    // Settings
    if (mouse_x >= 10 && mouse_x <= 210 && mouse_y >= (int32_t)fb_height - 170 && mouse_y <= (int32_t)fb_height - 150) {
        fb_rect(15, fb_height - 175, 190, 25, hover_bg);
        fb_print("Settings", 25, fb_height - 170, text_col, hover_bg);
    } else {
        fb_print("Settings", 25, fb_height - 170, text_col, bg);
    }
    
    fb_rect(20, fb_height - 110, 180, 2, 0x7F8C8D);
    
    // Power Off
    if (mouse_x >= 10 && mouse_x <= 210 && mouse_y >= (int32_t)fb_height - 90 && mouse_y <= (int32_t)fb_height - 70) {
        fb_rect(15, fb_height - 95, 190, 25, 0xE74C3C);
        fb_print("Power Off", 25, fb_height - 90, text_col, 0xE74C3C);
    } else {
        fb_print("Power Off", 25, fb_height - 90, 0xE74C3C, bg);
    }
}

static void draw_task_switcher() {
    uint32_t cx = (fb_width - 300) / 2;
    uint32_t cy = (fb_height - 150) / 2;
    
    fb_rect(cx, cy, 300, 150, ui_theme == 1 ? 0xFFFFFF : 0x222222);
    fb_rect(cx+2, cy+2, 296, 146, ui_theme == 1 ? 0xC0C0C0 : 0x444444);
    
    fb_print("Task Switcher", cx + 100, cy + 20, ui_theme == 1 ? 0x000080 : 0xFFFFFF, ui_theme == 1 ? 0xC0C0C0 : 0x444444);
    
    window_t* active_w = wm_get_active();
    if (active_w) {
        fb_print("Active Window:", cx + 20, cy + 60, 0xFFFFFF, ui_theme == 1 ? 0xC0C0C0 : 0x444444);
        fb_print(active_w->title, cx + 20, cy + 80, 0xF5A623, ui_theme == 1 ? 0xC0C0C0 : 0x444444);
    }
}

void shell_loop(void) {
    vfs_init();
    wm_init();
    
    // Initialize Registry and Apps
    app_registry_init();
    htop_init();
    yano_init();
    sysinfo_init();
    browser_init();
    files_init();
    settings_init();
    terminal_init();
    
    // Launch terminal app as standard boot window
    app_entry_t* term = app_find("terminal");
    if (term) term->launch(0);

    int full_redraw = 1;
    int32_t last_mx = -1, last_my = -1;
    uint8_t last_mbl = 0, last_mbr = 0;

    while(1) {
        // Alt-Tab Task Switcher
        if (ps2_is_alt_pressed()) {
            if (!task_switcher_visible) {
                task_switcher_visible = 1;
                full_redraw = 1;
            }
        } else {
            if (task_switcher_visible) {
                task_switcher_visible = 0;
                full_redraw = 1;
            }
        }

        // Check if anything besides mouse movement happened
        if (mouse_b_left != last_mbl || mouse_b_right != last_mbr || mouse_b_left || mouse_b_right) {
            full_redraw = 1;
        }

        wm_handle_mouse(mouse_x, mouse_y, mouse_b_left);
        
        // Start Menu Hover requires UI redraw
        if (start_menu_visible && (mouse_x != last_mx || mouse_y != last_my)) {
            if (mouse_x >= 10 && mouse_x <= 210 && mouse_y >= (int32_t)fb_height - 350 && mouse_y <= (int32_t)fb_height - 50) {
                full_redraw = 1;
            }
        }

        if (mouse_b_left && mouse_b_left != last_mbl) {
            if (menu_visible) {
                menu_visible = 0;
                full_redraw = 1;
            }
            
            if (mouse_x >= 10 && mouse_x <= 110 &&
                mouse_y >= (int32_t)fb_height - 38 && mouse_y <= (int32_t)fb_height - 8) {
                start_menu_visible = !start_menu_visible;
                full_redraw = 1;
            } else {
            if (start_menu_visible && mouse_x >= 10 && mouse_x <= 210 && mouse_y >= (int32_t)fb_height - 350 && mouse_y <= (int32_t)fb_height - 50) {
                    if (mouse_y >= (int32_t)fb_height - 290 && mouse_y <= (int32_t)fb_height - 270) { 
                        app_entry_t* app = app_find("browser");
                        if (app) app->launch(0);
                    } else if (mouse_y >= (int32_t)fb_height - 250 && mouse_y <= (int32_t)fb_height - 230) {
                        app_entry_t* app = app_find("sysinfo");
                        if (app) app->launch(0);
                    } else if (mouse_y >= (int32_t)fb_height - 210 && mouse_y <= (int32_t)fb_height - 190) { 
                        app_entry_t* app = app_find("files");
                        if (app) app->launch(0);
                    } else if (mouse_y >= (int32_t)fb_height - 170 && mouse_y <= (int32_t)fb_height - 150) { 
                        app_entry_t* app = app_find("settings");
                        if (app) app->launch(0);
                    } else if (mouse_y >= (int32_t)fb_height - 90 && mouse_y <= (int32_t)fb_height - 70) {
                        outw(0x604, 0x2000); // QEMU ACPI shutdown
                        outw(0xB004, 0x2000); // Bochs shutdown fallback
                    }
                    start_menu_visible = 0;
                    full_redraw = 1;
                }
                
                window_t* active_w = wm_get_active();
                if (active_w && !active_w->closed && strcmp(active_w->title, "Settings") == 0) {
                    if (mouse_x >= active_w->x + 20 && mouse_x <= active_w->x + 220) {
                        if (mouse_y >= active_w->y + 70 && mouse_y <= active_w->y + 100) {
                            ui_theme = 1; fb_render_wallpaper(); full_redraw = 1;
                        } else if (mouse_y >= active_w->y + 110 && mouse_y <= active_w->y + 140) {
                            ui_theme = 0; fb_render_wallpaper(); full_redraw = 1;
                        } else if (mouse_y >= active_w->y + 190 && mouse_y <= active_w->y + 220) {
                            wallpaper_mode = 1; fb_render_wallpaper(); full_redraw = 1;
                        } else if (mouse_y >= active_w->y + 230 && mouse_y <= active_w->y + 260) {
                            wallpaper_mode = 0; fb_render_wallpaper(); full_redraw = 1;
                        }
                    }
                }

                // Desktop double-click launcher
                for(int i = 0; i < 3; i++) {
                    if (mouse_x >= icons[i].x && mouse_x <= icons[i].x + icons[i].w &&
                        mouse_y >= icons[i].y && mouse_y <= icons[i].y + icons[i].h) {
                        extern volatile uint64_t kernel_ticks;
                        if (kernel_ticks - last_click_time < 30 && i == last_click_x) { 
                            app_entry_t* app = app_find(icons[i].app_name);
                            if (app) app->launch(0);
                            start_menu_visible = 0;
                            full_redraw = 1;
                        }
                        last_click_time = kernel_ticks;
                        last_click_x = i;
                    }
                }
                
                if (start_menu_visible && mouse_x > 210) {
                    start_menu_visible = 0;
                    full_redraw = 1;
                }
            }
        }

        if (mouse_b_right && mouse_b_right != last_mbr) {
            menu_x = mouse_x; menu_y = mouse_y;
            menu_visible = 1; full_redraw = 1;
        }

        // Keystroke Dispatcher Layer
        char c = ps2_get_last_key();
        window_t* active_w = wm_get_active();
        
        if (c) {
            full_redraw = 1;
            if (active_w && !active_w->closed) {
                // Find matching registered application for active window title
                app_entry_t* app = app_find_by_title(active_w->title);
                if (app && app->handle_key) {
                    app->handle_key(c); // Dispatched!
                }
            }
        }

        // Composite Desktop Render Pass
        if (full_redraw) {
            fb_clear_to_wallpaper();
            for(int i = 0; i < 3; i++) draw_icon(&icons[i]);
            wm_draw_all();
            fb_rect(0, fb_height - 45, fb_width, 45, ui_theme == 1 ? 0xC0C0C0 : 0x1A1A1A);
            
            // Start button highlight
            if (mouse_x >= 10 && mouse_x <= 110 && mouse_y >= (int32_t)fb_height - 38 && mouse_y <= (int32_t)fb_height - 8) {
                fb_rect(10, fb_height - 38, 100, 30, ui_theme == 1 ? 0xFFFFFF : 0x4A90E2);
            } else {
                fb_rect(10, fb_height - 38, 100, 30, ui_theme == 1 ? 0x808080 : 0x2C3E50);
            }
            
            fb_print("START", 35, fb_height - 30, ui_theme == 1 ? 0x000000 : 0xFFFFFF, ui_theme == 1 ? (mouse_x >= 10 && mouse_x <= 110 && mouse_y >= (int32_t)fb_height - 38 && mouse_y <= (int32_t)fb_height - 8 ? 0xFFFFFF : 0x808080) : (mouse_x >= 10 && mouse_x <= 110 && mouse_y >= (int32_t)fb_height - 38 && mouse_y <= (int32_t)fb_height - 8 ? 0x4A90E2 : 0x2C3E50));
            fb_print("01:30 AM", fb_width - 80, fb_height - 30, ui_theme == 1 ? 0x000000 : 0xAAAAAA, ui_theme == 1 ? 0xC0C0C0 : 0x1A1A1A);

            if (menu_visible) draw_context_menu(menu_x, menu_y);
            if (start_menu_visible) draw_start_menu();
            if (task_switcher_visible) draw_task_switcher();

            // Cache the clean desktop before cursor
            fb_commit_desktop();

            fb_draw_cursor(mouse_x, mouse_y);
            fb_swap();
        } else if (mouse_x != last_mx || mouse_y != last_my) {
            // ONLY MOUSE MOVED! Zero CPU usage path!
            if (last_mx >= 0 && last_my >= 0) {
                fb_restore_desktop_rect(last_mx, last_my, 16, 16);
            }
            fb_draw_cursor(mouse_x, mouse_y);
            fb_swap();
        }

        last_mx = mouse_x;
        last_my = mouse_y;
        last_mbl = mouse_b_left;
        last_mbr = mouse_b_right;
        
        // Brief sleep to avoid pegging host CPU in virtual environments
        sleep(5);
    }
}