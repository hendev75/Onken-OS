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

int ui_theme = 1; // 1 = Retro (Workbench), 0 = Modern
int wallpaper_mode = 1; // 1 = Teal Solid, 0 = Space Gradient

// Icons on desktop (Draggable)
typedef struct {
    int x, y, w, h;
    const char* label;
    const char* app_name;
    uint32_t color;
} icon_t;

static icon_t icons[] = {
    {60, 60, 64, 64, "Browser", "browser", 0x4A90E2},
    {60, 160, 64, 64, "Terminal", "terminal", 0x50E3C2},
    {60, 260, 64, 64, "Files", "files", 0xF5A623}
};

// Start Menu & Desktop drag states
static int start_menu_visible = 0;
static int menu_visible = 0;
static int menu_x = 0, menu_y = 0;

static uint32_t last_click_time = 0;
static int last_click_x = -1;

static int task_switcher_visible = 0;

// Icon dragging states
static int dragging_icon_idx = -1;
static int drag_icon_off_x = 0, drag_icon_off_y = 0;

static void draw_icon(icon_t* icon) {
    if (ui_theme == 1) {
        // Draw highly premium retro 3D folder icon
        draw_retro_3d_panel(icon->x, icon->y, icon->w, icon->h, 0);
        
        // Folder tab
        fb_rect(icon->x + 8, icon->y + 8, 16, 4, 0x000000);
        fb_rect(icon->x + 9, icon->y + 9, 14, 3, 0xD4A017); // Gold tab
        
        // Folder body bevel
        fb_rect(icon->x + 8, icon->y + 12, icon->w - 16, icon->h - 24, 0x000000);
        draw_retro_3d_panel(icon->x + 9, icon->y + 13, icon->w - 18, icon->h - 26, 0);
        fb_rect(icon->x + 11, icon->y + 15, icon->w - 22, icon->h - 30, 0xF3C34F); // Gold inside
        
        // Label with shadow box
        int len = strlen(icon->label) * 8;
        int lx = icon->x + (icon->w - len)/2;
        fb_rect(lx - 4, icon->y + icon->h + 2, len + 8, 12, 0x000000);
        fb_rect(lx - 3, icon->y + icon->h + 3, len + 6, 10, 0xC0C0C0);
        fb_print(icon->label, lx, icon->y + icon->h + 4, 0x000000, 0xC0C0C0);
    } else {
        // Flat modern icon
        fb_rect(icon->x, icon->y, icon->w, icon->h, icon->color);
        fb_rect(icon->x + 4, icon->y + 4, icon->w - 8, icon->h - 8, 0xFFFFFF);
        fb_rect(icon->x + 8, icon->y + 8, icon->w - 16, icon->h - 16, 0x222222);
        fb_print(icon->label, icon->x + (icon->w - strlen(icon->label)*8)/2, icon->y + icon->h + 8, 0xFFFFFF, 0x000000);
    }
}

static void draw_context_menu(int x, int y) {
    if (ui_theme == 1) {
        draw_retro_3d_panel(x, y, 150, 80, 0);
        fb_print("Refresh", x + 15, y + 12, 0x000000, 0xC0C0C0);
        fb_print("Wallpaper", x + 15, y + 36, 0x000000, 0xC0C0C0);
        fb_print("Settings", x + 15, y + 60, 0x000000, 0xC0C0C0);
    } else {
        fb_rect(x, y, 150, 80, 0x222222);
        fb_rect(x+1, y+1, 148, 78, 0x444444);
        fb_print("Refresh", x + 15, y + 12, 0xFFFFFF, 0x444444);
        fb_print("Wallpaper", x + 15, y + 36, 0xFFFFFF, 0x444444);
        fb_print("Settings", x + 15, y + 60, 0xFFFFFF, 0x444444);
    }
}

extern int32_t mouse_x;
extern int32_t mouse_y;

static void draw_start_menu() {
    uint32_t bg = ui_theme == 1 ? 0xC0C0C0 : 0x34495E;
    uint32_t hover_bg = ui_theme == 1 ? 0x224488 : 0x4A90E2; 
    uint32_t text_col = 0xFFFFFF;
    
    if (ui_theme == 1) {
        draw_retro_3d_panel(10, fb_height - 350, 200, 300, 0);
    } else {
        fb_rect(10, fb_height - 350, 200, 300, 0x222222);
        fb_rect(11, fb_height - 349, 198, 298, bg);
    }
    
    fb_print("Onken OS AROS", 20, fb_height - 330, ui_theme == 1 ? 0x224488 : 0xF5A623, bg);
    fb_rect(20, fb_height - 310, 180, 2, 0x7F8C8D);
    
    // Programs
    if (mouse_x >= 10 && mouse_x <= 210 && mouse_y >= (int32_t)fb_height - 290 && mouse_y <= (int32_t)fb_height - 270) {
        fb_rect(15, fb_height - 295, 190, 25, hover_bg);
        fb_print("Programs", 25, fb_height - 290, text_col, hover_bg);
    } else {
        fb_print("Programs", 25, fb_height - 290, ui_theme == 1 ? 0x000000 : text_col, bg);
    }

    // System Info
    if (mouse_x >= 10 && mouse_x <= 210 && mouse_y >= (int32_t)fb_height - 250 && mouse_y <= (int32_t)fb_height - 230) {
        fb_rect(15, fb_height - 255, 190, 25, hover_bg);
        fb_print("System Info", 25, fb_height - 250, text_col, hover_bg);
    } else {
        fb_print("System Info", 25, fb_height - 250, ui_theme == 1 ? 0x000000 : text_col, bg);
    }

    // Filesystem
    if (mouse_x >= 10 && mouse_x <= 210 && mouse_y >= (int32_t)fb_height - 210 && mouse_y <= (int32_t)fb_height - 190) {
        fb_rect(15, fb_height - 215, 190, 25, hover_bg);
        fb_print("Filesystem", 25, fb_height - 210, text_col, hover_bg);
    } else {
        fb_print("Filesystem", 25, fb_height - 210, ui_theme == 1 ? 0x000000 : text_col, bg);
    }

    // Settings
    if (mouse_x >= 10 && mouse_x <= 210 && mouse_y >= (int32_t)fb_height - 170 && mouse_y <= (int32_t)fb_height - 150) {
        fb_rect(15, fb_height - 175, 190, 25, hover_bg);
        fb_print("Settings", 25, fb_height - 170, text_col, hover_bg);
    } else {
        fb_print("Settings", 25, fb_height - 170, ui_theme == 1 ? 0x000000 : text_col, bg);
    }
    
    fb_rect(20, fb_height - 110, 180, 2, 0x7F8C8D);
    
    // Power Off
    if (mouse_x >= 10 && mouse_x <= 210 && mouse_y >= (int32_t)fb_height - 90 && mouse_y <= (int32_t)fb_height - 70) {
        fb_rect(15, fb_height - 95, 190, 25, 0xBB3333);
        fb_print("Power Off", 25, fb_height - 90, text_col, 0xBB3333);
    } else {
        fb_print("Power Off", 25, fb_height - 90, 0xBB3333, bg);
    }
}

static void draw_task_switcher() {
    uint32_t cx = (fb_width - 300) / 2;
    uint32_t cy = (fb_height - 150) / 2;
    
    if (ui_theme == 1) {
        draw_retro_3d_panel(cx, cy, 300, 150, 0);
        fb_print("Task Switcher", cx + 90, cy + 20, 0x224488, 0xC0C0C0);
    } else {
        fb_rect(cx, cy, 300, 150, 0x222222);
        fb_rect(cx+2, cy+2, 296, 146, 0x444444);
        fb_print("Task Switcher", cx + 90, cy + 20, 0xFFFFFF, 0x444444);
    }
    
    window_t* active_w = wm_get_active();
    if (active_w) {
        fb_print("Active Window:", cx + 20, cy + 60, ui_theme == 1 ? 0x000000 : 0xFFFFFF, ui_theme == 1 ? 0xC0C0C0 : 0x444444);
        fb_print(active_w->title, cx + 20, cy + 80, 0xD4A017, ui_theme == 1 ? 0xC0C0C0 : 0x444444);
    }
}

// Workbench top status bar
static void draw_workbench_bar() {
    if (ui_theme == 1) {
        draw_retro_3d_panel(0, 0, fb_width, 22, 0);
        
        // Workbench Logo
        fb_print("A", 10, 6, 0xBB3333, 0xC0C0C0);
        fb_print("OnkenOS Workbench v3.0", 30, 6, 0x000000, 0xC0C0C0);
        
        // Subsystem values
        char stats[128];
        uint32_t uptime_s = (uint32_t)(sys_uptime() / 1000);
        
        // Fetch real memory info
        mem_info_t mem;
        sys_get_mem_info(&mem);
        uint32_t free_mb = (uint32_t)((mem.total_memory - mem.used_memory) / 1024 / 1024);
        
        xsprintf(stats, "RAM Free: %d MB  |  Uptime: %d s  |  Active: %s", 
                 free_mb, uptime_s, wm_get_active() ? wm_get_active()->title : "None");
        
        fb_print(stats, fb_width - 500, 6, 0x224488, 0xC0C0C0);
    } else {
        fb_rect(0, 0, fb_width, 22, 0x111111);
        fb_print("Onken OS (Modern)", 15, 6, 0xFFFFFF, 0x111111);
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

        // Change tracking
        if (mouse_b_left != last_mbl || mouse_b_right != last_mbr || mouse_b_left || mouse_b_right) {
            full_redraw = 1;
        }

        // 1. Desktop Icon dragging logic
        if (mouse_b_left) {
            if (dragging_icon_idx != -1) {
                icons[dragging_icon_idx].x = mouse_x - drag_icon_off_x;
                icons[dragging_icon_idx].y = mouse_y - drag_icon_off_y;
                
                // Boundaries keeping inside screen safely
                if (icons[dragging_icon_idx].x < 10) icons[dragging_icon_idx].x = 10;
                if (icons[dragging_icon_idx].y < 26) icons[dragging_icon_idx].y = 26; // Below screen titlebar
                if (icons[dragging_icon_idx].x > (int)fb_width - 80) icons[dragging_icon_idx].x = fb_width - 80;
                if (icons[dragging_icon_idx].y > (int)fb_height - 120) icons[dragging_icon_idx].y = fb_height - 120;
                full_redraw = 1;
            } else {
                // Check if clicking on an icon when no window gets clicked
                window_t* act = wm_get_active();
                int clicked_window = 0;
                if (act && !act->closed) {
                    uint32_t ch = act->minimized ? 24 : act->h;
                    if (mouse_x >= (int)act->x && mouse_x <= (int)(act->x + act->w) &&
                        mouse_y >= (int)act->y && mouse_y <= (int)(act->y + ch)) {
                        clicked_window = 1;
                    }
                }
                
                if (!clicked_window && mouse_b_left != last_mbl) {
                    for (int i = 0; i < 3; i++) {
                        if (mouse_x >= icons[i].x && mouse_x <= icons[i].x + icons[i].w &&
                            mouse_y >= icons[i].y && mouse_y <= icons[i].y + icons[i].h) {
                            dragging_icon_idx = i;
                            drag_icon_off_x = mouse_x - icons[i].x;
                            drag_icon_off_y = mouse_y - icons[i].y;
                            break;
                        }
                    }
                }
            }
        } else {
            dragging_icon_idx = -1;
        }

        // Call mouse routing of window manager
        if (dragging_icon_idx == -1) {
            wm_handle_mouse(mouse_x, mouse_y, mouse_b_left);
        }
        
        // Start Menu Hover state redraw
        if (start_menu_visible && (mouse_x != last_mx || mouse_y != last_my)) {
            if (mouse_x >= 10 && mouse_x <= 210 && mouse_y >= (int32_t)fb_height - 350 && mouse_y <= (int32_t)fb_height - 50) {
                full_redraw = 1;
            }
        }

        if (mouse_b_left && mouse_b_left != last_mbl) {
            // Check context menu clicks first if visible
            if (menu_visible) {
                if (mouse_x >= menu_x && mouse_x <= menu_x + 150 &&
                    mouse_y >= menu_y && mouse_y <= menu_y + 80) {
                    int click_offset_y = mouse_y - menu_y;
                    if (click_offset_y >= 5 && click_offset_y <= 28) {
                        // Refresh
                        full_redraw = 1;
                    } else if (click_offset_y >= 29 && click_offset_y <= 52) {
                        // Wallpaper toggle
                        extern int wallpaper_mode;
                        wallpaper_mode = (wallpaper_mode == 1) ? 0 : 1;
                        fb_render_wallpaper();
                        full_redraw = 1;
                    } else if (click_offset_y >= 53 && click_offset_y <= 75) {
                        // Settings
                        app_entry_t* app = app_find("settings");
                        if (app) app->launch(0);
                    }
                }
                menu_visible = 0;
                full_redraw = 1;
            }
            
            // Check double-click launcher
            int double_clicked = 0;
            for(int i = 0; i < 3; i++) {
                if (mouse_x >= icons[i].x && mouse_x <= icons[i].x + icons[i].w &&
                    mouse_y >= icons[i].y && mouse_y <= icons[i].y + icons[i].h) {
                    extern volatile uint64_t kernel_ticks;
                    if (kernel_ticks - last_click_time < 80 && i == last_click_x) { 
                        app_entry_t* app = app_find(icons[i].app_name);
                        if (app) app->launch(0);
                        start_menu_visible = 0;
                        full_redraw = 1;
                        double_clicked = 1;
                    }
                    last_click_time = kernel_ticks;
                    last_click_x = i;
                }
            }
            
            if (!double_clicked) {
                // Check Start Menu trigger button
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
                    
                    // Appearance Settings dynamic click handler
                    window_t* active_w = wm_get_active();
                    if (active_w && !active_w->closed && strcmp(active_w->title, "Settings") == 0) {
                        if (mouse_x >= (int)active_w->x + 20 && mouse_x <= (int)active_w->x + 220) {
                            if (mouse_y >= (int)active_w->y + 70 && mouse_y <= (int)active_w->y + 100) {
                                ui_theme = 1; fb_render_wallpaper(); full_redraw = 1;
                            } else if (mouse_y >= (int)active_w->y + 110 && mouse_y <= (int)active_w->y + 140) {
                                ui_theme = 0; fb_render_wallpaper(); full_redraw = 1;
                            } else if (mouse_y >= (int)active_w->y + 190 && mouse_y <= (int)active_w->y + 220) {
                                wallpaper_mode = 1; fb_render_wallpaper(); full_redraw = 1;
                            } else if (mouse_y >= (int)active_w->y + 230 && mouse_y <= (int)active_w->y + 260) {
                                wallpaper_mode = 0; fb_render_wallpaper(); full_redraw = 1;
                            }
                        }
                    }
                    
                    if (start_menu_visible && mouse_x > 210) {
                        start_menu_visible = 0;
                        full_redraw = 1;
                    }
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
                app_entry_t* app = app_find_by_title(active_w->title);
                if (app && app->handle_key) {
                    app->handle_key(c); 
                }
            }
        }

        // Desktop Compositing Render pass
        if (full_redraw) {
            fb_reset_cursor_backup();
            fb_clear_to_wallpaper();
            
            // Draw desktop icons
            for(int i = 0; i < 3; i++) draw_icon(&icons[i]);
            
            // Draw window manager layers
            wm_draw_all();
            
            // Draw screen menu bar
            draw_workbench_bar();
            
            // Bottom Taskbar (AROS 3D look)
            if (ui_theme == 1) {
                draw_retro_3d_panel(0, fb_height - 45, fb_width, 45, 0);
            } else {
                fb_rect(0, fb_height - 45, fb_width, 45, 0x1A1A1A);
            }
            
            // START button highlight / 3D look
            if (mouse_x >= 10 && mouse_x <= 110 && mouse_y >= (int32_t)fb_height - 38 && mouse_y <= (int32_t)fb_height - 8) {
                if (ui_theme == 1) {
                    draw_retro_3d_panel(10, fb_height - 38, 100, 30, 1); // Sunken when hovering
                } else {
                    fb_rect(10, fb_height - 38, 100, 30, 0x4A90E2);
                }
            } else {
                if (ui_theme == 1) {
                    draw_retro_3d_panel(10, fb_height - 38, 100, 30, 0); // Raised normal
                } else {
                    fb_rect(10, fb_height - 38, 100, 30, 0x2C3E50);
                }
            }
            
            fb_print("START", 35, fb_height - 28, ui_theme == 1 ? 0x000000 : 0xFFFFFF, ui_theme == 1 ? 0xC0C0C0 : (mouse_x >= 10 && mouse_x <= 110 && mouse_y >= (int32_t)fb_height - 38 && mouse_y <= (int32_t)fb_height - 8 ? 0x4A90E2 : 0x2C3E50));
            fb_print("01:30 AM", fb_width - 80, fb_height - 28, ui_theme == 1 ? 0x000000 : 0xAAAAAA, ui_theme == 1 ? 0xC0C0C0 : 0x1A1A1A);

            if (menu_visible) draw_context_menu(menu_x, menu_y);
            if (start_menu_visible) draw_start_menu();
            if (task_switcher_visible) draw_task_switcher();

            // Cache desktop surface before cursor
            fb_commit_desktop();

            fb_show_cursor(mouse_x, mouse_y);
            fb_swap();
            
            full_redraw = 0; // successfully finished redraw
        } else if (mouse_x != last_mx || mouse_y != last_my) {
            // zero CPU mouse motion pass
            fb_show_cursor(mouse_x, mouse_y);
            fb_swap();
        }

        last_mx = mouse_x;
        last_my = mouse_y;
        last_mbl = mouse_b_left;
        last_mbr = mouse_b_right;
        
        sleep(5);
    }
}