#include "../kernel/kernel.h"
#include "../gui/fb.h"
#include "../drivers/ps2.h"
#include "../kernel/string.h"
#include "../kernel/fs.h"
#include "../gui/window.h"

int ui_theme = 1; // 1 = Retro, 0 = Modern
int wallpaper_mode = 1; // 0 = Gradient, 1 = Teal, 2 = Stars

#define MAX_CMD 128
static char cmd_buf[MAX_CMD];
static uint32_t cmd_len = 0;
static char current_dir[64] = "/";

static char yano_buf[1024] = "This is yano (yet another nano).\nThe ultra-fast text editor for Onken OS.\nStart typing here...";
static uint32_t yano_len = 86;

// Terminal History
#define MAX_HISTORY 20
typedef struct {
    char text[128];
    uint32_t color;
} history_line_t;
static history_line_t term_history[MAX_HISTORY];
static int history_count = 0;

static void term_history_add(const char* msg, uint32_t color) {
    if (history_count < MAX_HISTORY) {
        strncpy(term_history[history_count].text, msg, 127);
        term_history[history_count].color = color;
        history_count++;
    } else {
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            term_history[i] = term_history[i + 1];
        }
        strncpy(term_history[MAX_HISTORY - 1].text, msg, 127);
        term_history[MAX_HISTORY - 1].color = color;
    }
}

// Icons
typedef struct {
    int x, y, w, h;
    const char* label;
    uint32_t color;
} icon_t;

static icon_t icons[] = {
    {40, 40, 64, 64, "Browser", 0x4A90E2},
    {40, 140, 64, 64, "Terminal", 0x50E3C2},
    {40, 240, 64, 64, "Files", 0xF5A623}
};

// Start Menu
static int start_menu_visible = 0;
static int menu_visible = 0;
static int menu_x = 0, menu_y = 0;

static uint32_t last_click_time = 0;
static int last_click_x = -1;
static uint32_t global_ticks = 0;

static int task_switcher_visible = 0;

void draw_terminal_content(void* self);
void draw_browser_content(void* self);
void draw_files_content(void* self);
void draw_settings_content(void* self);
void draw_sysinfo_content(void* self);
void draw_htop_content(void* self);
void draw_yano_content(void* self);

static void print_to_shell(window_t* w, const char* msg, uint32_t color) {
    (void)w;
    term_history_add(msg, color);
}

void draw_terminal_content(void* self) {
    window_t* w = (window_t*)self;
    uint32_t sy = w->y + 45;
    
    for (int i = 0; i < history_count; i++) {
        fb_print(term_history[i].text, w->x + 15, sy, term_history[i].color, 0x111111);
        sy += 12;
    }
    
    // Draw prompt
    fb_print("root@onken:", w->x + 15, sy, 0x50E3C2, 0x111111);
    fb_print(current_dir, w->x + 15 + 11*8, sy, 0xFFFFFF, 0x111111);
    fb_print("$ ", w->x + 15 + (11 + strlen(current_dir))*8, sy, 0xBD10E0, 0x111111);
    
    // Draw current command buffer
    fb_print(cmd_buf, w->x + 15 + (13 + strlen(current_dir))*8, sy, 0xFFFFFF, 0x111111);
}

void draw_browser_content(void* self) {
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

void draw_files_content(void* self) {
    window_t* w = (window_t*)self;
    fb_print("System Drive (C:)", w->x + 20, w->y + 40, 0xF5A623, 0x111111);
    
    fb_print("[DIR] boot/", w->x + 20, w->y + 70, 0x4A90E2, 0x111111);
    fb_print("[DIR] drivers/", w->x + 20, w->y + 90, 0x4A90E2, 0x111111);
    fb_print("[DIR] gui/", w->x + 20, w->y + 110, 0x4A90E2, 0x111111);
    fb_print("[DIR] kernel/", w->x + 20, w->y + 130, 0x4A90E2, 0x111111);
    fb_print("[DIR] shell/", w->x + 20, w->y + 150, 0x4A90E2, 0x111111);
    
    fb_print("[FILE] Makefile", w->x + 200, w->y + 70, 0xBDC3C7, 0x111111);
    fb_print("[FILE] limine.conf", w->x + 200, w->y + 90, 0xBDC3C7, 0x111111);
    fb_print("[FILE] readme.txt", w->x + 200, w->y + 110, 0xBDC3C7, 0x111111);
}

void draw_settings_content(void* self) {
    window_t* w = (window_t*)self;
    fb_print("Appearance Settings", w->x + 20, w->y + 40, 0xFFFFFF, 0x111111);
    
    fb_rect(w->x + 20, w->y + 70, 200, 30, ui_theme == 1 ? 0x008080 : 0x444444);
    fb_print("Retro Theme", w->x + 30, w->y + 80, 0xFFFFFF, ui_theme == 1 ? 0x008080 : 0x444444);
    
    fb_rect(w->x + 20, w->y + 110, 200, 30, ui_theme == 0 ? 0x4A90E2 : 0x444444);
    fb_print("Modern Theme", w->x + 30, w->y + 120, 0xFFFFFF, ui_theme == 0 ? 0x4A90E2 : 0x444444);

    fb_print("Wallpaper Settings", w->x + 20, w->y + 160, 0xFFFFFF, 0x111111);

    fb_rect(w->x + 20, w->y + 190, 200, 30, wallpaper_mode == 1 ? 0x008080 : 0x444444);
    fb_print("Solid Teal", w->x + 30, w->y + 200, 0xFFFFFF, wallpaper_mode == 1 ? 0x008080 : 0x444444);

    fb_rect(w->x + 20, w->y + 230, 200, 30, wallpaper_mode == 0 ? 0x4A90E2 : 0x444444);
    fb_print("BoredOS Space", w->x + 30, w->y + 240, 0xFFFFFF, wallpaper_mode == 0 ? 0x4A90E2 : 0x444444);
}

void draw_sysinfo_content(void* self) {
    window_t* w = (window_t*)self;
    
    // ASCII Banner
    fb_print("  ____  _   _ _  _______ _   _ ", w->x + 20, w->y + 40, 0x50E3C2, 0x111111);
    fb_print(" / __ \\| \\ | | |/ / ____| \\ | |", w->x + 20, w->y + 52, 0x50E3C2, 0x111111);
    fb_print("| |  | |  \\| | ' /| |__ |  \\| |", w->x + 20, w->y + 64, 0x50E3C2, 0x111111);
    fb_print("| |  | | . ` |  < |  __|| . ` |", w->x + 20, w->y + 76, 0x50E3C2, 0x111111);
    fb_print("| |__| | |\\  | . \\| |___| |\\  |", w->x + 20, w->y + 88, 0x50E3C2, 0x111111);
    fb_print(" \\____/|_| \\_|_|\\_\\_____|_| \\_|", w->x + 20, w->y + 100, 0x50E3C2, 0x111111);

    fb_print("Made by Ayham & Mason", w->x + 20, w->y + 130, 0xF5A623, 0x111111);
    fb_print("OS Version  : Onken OS Revolution v3.0.0", w->x + 20, w->y + 160, 0xFFFFFF, 0x111111);
    fb_print("Kernel      : Onken Microkernel x86_64", w->x + 20, w->y + 175, 0xFFFFFF, 0x111111);
    fb_print("Graphics    : Desktop Compositor Engine", w->x + 20, w->y + 190, 0xFFFFFF, 0x111111);
    fb_print("Memory Map  : HHDM via Limine Protocol", w->x + 20, w->y + 205, 0xFFFFFF, 0x111111);
}

void draw_htop_content(void* self) {
    window_t* w = (window_t*)self;
    fb_print("htop - Task Manager", w->x + 10, w->y + 35, 0x00FF00, 0x111111);
    
    // CPU 1 Bar
    fb_print("CPU1 [||||||||||       ] 48.0%", w->x + 10, w->y + 60, 0x4A90E2, 0x111111);
    fb_print("CPU2 [||||             ] 21.0%", w->x + 10, w->y + 75, 0x4A90E2, 0x111111);
    fb_print("Mem  [||||||||         ] 128MB/512MB", w->x + 10, w->y + 90, 0xF5A623, 0x111111);
    fb_print("Swp  [                 ] 0K/0K", w->x + 10, w->y + 105, 0xBD10E0, 0x111111);

    fb_rect(w->x + 10, w->y + 130, w->w - 20, 2, 0x555555);
    
    fb_print("  PID USER   PR  NI   VIRT   RES  SHR S %CPU %MEM   TIME+ COMMAND", w->x + 10, w->y + 140, 0xFFFFFF, 0x111111);
    fb_print("    1 root   20   0   2.5M  1.2M    0 S  0.0  0.2 0:01.00 init", w->x + 10, w->y + 155, 0xCCCCCC, 0x111111);
    fb_print("   42 root   20   0   8.0M  4.0M    0 R 35.5  0.8 0:05.23 window_manag", w->x + 10, w->y + 170, 0x00FF00, 0x111111);
    fb_print("   43 root   20   0   4.2M  1.5M    0 S  2.0  0.3 0:00.50 terminal", w->x + 10, w->y + 185, 0xCCCCCC, 0x111111);
    fb_print("   60 ayham  20   0   1.0M  0.5M    0 R  5.0  0.1 0:00.10 htop", w->x + 10, w->y + 200, 0xCCCCCC, 0x111111);
}

void draw_yano_content(void* self) {
    window_t* w = (window_t*)self;
    // Yano header
    fb_rect(w->x + 2, w->y + 22, w->w - 4, 16, 0xDDDDDD);
    fb_print("  UW PICO 5.09               File: new.txt", w->x + 10, w->y + 26, 0x000000, 0xDDDDDD);
    
    // Content area
    int cx = 0, cy = 0;
    for (uint32_t i = 0; i < yano_len; i++) {
        if (yano_buf[i] == '\n') {
            cy += 15;
            cx = 0;
        } else {
            char temp[2] = {yano_buf[i], '\0'};
            fb_print(temp, w->x + 10 + cx, w->y + 50 + cy, 0xFFFFFF, 0x111111);
            cx += 8;
        }
    }

    // Cursor
    fb_rect(w->x + 10 + cx, w->y + 50 + cy, 8, 12, 0xFFFFFF);

    // Yano footer
    fb_rect(w->x + 2, w->y + w->h - 32, w->w - 4, 30, 0xDDDDDD);
    fb_print("^G Get Help  ^O WriteOut  ^R Read File ^Y Prev Pg", w->x + 10, w->y + w->h - 28, 0x000000, 0xDDDDDD);
    fb_print("^X Exit      ^J Justify   ^W Where is  ^V Next Pg", w->x + 10, w->y + w->h - 16, 0x000000, 0xDDDDDD);
}

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
    
    // Programs
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

static void exec_command(window_t* w, const char* cmd) {
    char temp[MAX_CMD];
    strncpy(temp, cmd, MAX_CMD);
    char* argv0 = strtok(temp, " ");
    if (!argv0) return;

    if (strcmp(argv0, "help") == 0) {
        print_to_shell(w, "Available Commands:", 0x50E3C2);
        print_to_shell(w, "  cd    - Change directory", 0xFFFFFF);
        print_to_shell(w, "  ls    - List directory", 0xFFFFFF);
        print_to_shell(w, "  htop  - Task manager", 0xFFFFFF);
        print_to_shell(w, "  yano  - Text editor", 0xFFFFFF);
        print_to_shell(w, "  kernel- Kernel log", 0xFFFFFF);
        print_to_shell(w, "  ver   - OS version", 0xFFFFFF);
        print_to_shell(w, "  clear - Clear screen", 0xFFFFFF);
        print_to_shell(w, "  exit  - Close terminal", 0xFFFFFF);
    } else if (strcmp(argv0, "ls") == 0) {
        if (strcmp(current_dir, "/") == 0) {
            print_to_shell(w, "boot/     drivers/  gui/      kernel/", 0x4A90E2);
            print_to_shell(w, "shell/    Makefile  readme    onken.bin", 0xFFFFFF);
        } else {
            print_to_shell(w, "file1.txt  file2.txt", 0xFFFFFF);
        }
    } else if (strcmp(argv0, "cd") == 0) {
        char* arg1 = strtok(NULL, " ");
        if (arg1) {
            strncpy(current_dir, arg1, 63);
        } else {
            strncpy(current_dir, "/", 63);
        }
    } else if (strcmp(argv0, "kernel") == 0) {
        print_to_shell(w, "[0.00] Limine Bootloader protocol active", 0x50E3C2);
        print_to_shell(w, "[0.01] Initializing Memory Map...", 0x50E3C2);
        print_to_shell(w, "[0.05] HHDM offset: 0xFFFF800000000000", 0x50E3C2);
        print_to_shell(w, "[0.10] PS/2 Driver Loaded.", 0x50E3C2);
    } else if (strcmp(argv0, "htop") == 0) {
        wm_add_window(150, 150, 600, 300, "htop", draw_htop_content);
    } else if (strcmp(argv0, "yano") == 0) {
        wm_add_window(200, 200, 600, 400, "yano - file.txt", draw_yano_content);
    } else if (strcmp(argv0, "ver") == 0) {
        print_to_shell(w, "Onken OS Revolution v3.0.0", 0xF5A623);
    } else if (strcmp(argv0, "clear") == 0) {
        history_count = 0;
    } else if (strcmp(argv0, "exit") == 0) {
        w->closed = 1;
    } else {
        print_to_shell(w, "Command not found.", 0xD0021B);
    }
}

void shell_loop(void) {
    vfs_init();
    wm_init();
    
    wm_add_window(300, 200, 700, 500, "Onken Terminal", draw_terminal_content);

    int full_redraw = 1;
    int32_t last_mx = -1, last_my = -1;
    uint8_t last_mbl = 0, last_mbr = 0;

    while(1) {
        global_ticks++;
        ps2_poll();

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
                        wm_add_window(150, 100, 800, 500, "Onken Browser", draw_browser_content);
                    } else if (mouse_y >= (int32_t)fb_height - 250 && mouse_y <= (int32_t)fb_height - 230) {
                        wm_add_window(200, 100, 500, 300, "System Info", draw_sysinfo_content);
                    } else if (mouse_y >= (int32_t)fb_height - 210 && mouse_y <= (int32_t)fb_height - 190) { 
                        wm_add_window(200, 150, 600, 400, "File Explorer", draw_files_content);
                    } else if (mouse_y >= (int32_t)fb_height - 170 && mouse_y <= (int32_t)fb_height - 150) { 
                        wm_add_window(250, 200, 400, 300, "Settings", draw_settings_content);
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

                for(int i = 0; i < 3; i++) {
                    if (mouse_x >= icons[i].x && mouse_x <= icons[i].x + icons[i].w &&
                        mouse_y >= icons[i].y && mouse_y <= icons[i].y + icons[i].h) {
                        if (global_ticks - last_click_time < 30 && i == last_click_x) { 
                            if (i == 0) wm_add_window(150, 100, 800, 500, "Onken Browser - Search", draw_browser_content);
                            else if (i == 1) wm_add_window(350, 250, 700, 500, "Onken Terminal", draw_terminal_content);
                            else if (i == 2) wm_add_window(200, 150, 600, 400, "File Explorer", draw_files_content);
                            start_menu_visible = 0;
                            full_redraw = 1;
                        }
                        last_click_time = global_ticks;
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

        char c = ps2_get_last_key();
        window_t* active_w = wm_get_active();
        
        if (c) {
            full_redraw = 1;
            if (active_w && !active_w->closed) {
                if (strcmp(active_w->title, "Onken Terminal") == 0) {
                    if (c == '\n' || c == '\r') {
                        cmd_buf[cmd_len] = '\0';
                        char full_cmd[128];
                        strncpy(full_cmd, "root@onken:", 127);
                        uint32_t offset = strlen(full_cmd);
                        strncpy(full_cmd + offset, current_dir, 127 - offset);
                        offset = strlen(full_cmd);
                        strncpy(full_cmd + offset, "$ ", 127 - offset);
                        offset = strlen(full_cmd);
                        strncpy(full_cmd + offset, cmd_buf, 127 - offset);
                        full_cmd[127] = '\0';
                        term_history_add(full_cmd, 0xCCCCCC);
                        
                        exec_command(active_w, cmd_buf);
                        cmd_len = 0;
                        memset(cmd_buf, 0, MAX_CMD);
                    } else if (c == '\b') {
                        if (cmd_len > 0) cmd_buf[--cmd_len] = '\0';
                    } else if (c >= 32 && c < 127) {
                        if (cmd_len < MAX_CMD - 1) cmd_buf[cmd_len++] = c;
                    }
                } else if (strcmp(active_w->title, "yano - file.txt") == 0) {
                    if (c == '\n' || c == '\r') {
                        if (yano_len < 1023) yano_buf[yano_len++] = '\n';
                    } else if (c == '\b') {
                        if (yano_len > 0) yano_buf[--yano_len] = '\0';
                    } else if (c >= 32 && c < 127) {
                        if (yano_len < 1023) yano_buf[yano_len++] = c;
                    }
                }
            }
        }

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
    }
}