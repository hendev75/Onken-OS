#include "terminal.h"
#include "../../kernel/tasking/app.h"
#include "../../kernel/syscalls/syscall.h"
#include "../../gui/fb.h"
#include "../../gui/window.h"
#include "../../kernel/string.h"
#include "../../kernel/fs.h"
#include "../../kernel/memory/memory.h"
#include "../../kernel/time/pit.h"
#include "../../drivers/rtc.h"
#include "../../drivers/pci.h"

#define MAX_CMD 128
static char cmd_buf[MAX_CMD];
static uint32_t cmd_len = 0;
static char current_dir[64] = "/";

#define MAX_HISTORY 40
typedef struct {
    char text[128];
    uint32_t color;
} history_line_t;

static history_line_t term_history[MAX_HISTORY];
static int history_count = 0;

static app_entry_t terminal_app = {
    .name = "terminal",
    .title = "Onken Terminal",
    .description = "Command Shell",
    .init = terminal_init,
    .launch = terminal_launch,
    .draw = terminal_draw,
    .handle_key = terminal_handle_key
};

void term_history_add(const char* msg, uint32_t color) {
    if (history_count < MAX_HISTORY) {
        strncpy(term_history[history_count].text, msg, 127);
        term_history[history_count].text[127] = '\0';
        term_history[history_count].color = color;
        history_count++;
    } else {
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            term_history[i] = term_history[i + 1];
        }
        strncpy(term_history[MAX_HISTORY - 1].text, msg, 127);
        term_history[MAX_HISTORY - 1].text[127] = '\0';
        term_history[MAX_HISTORY - 1].color = color;
    }
}

static void show_fastfetch(void) {
    // Real system stats
    mem_info_t mem;
    sys_get_mem_info(&mem);
    uint32_t total_mb = (uint32_t)(mem.total_memory / 1024 / 1024);
    uint32_t used_mb = (uint32_t)(mem.used_memory / 1024 / 1024);
    uint32_t free_mb = total_mb - used_mb;
    
    // Real time from CMOS RTC
    int year, month, day, hour, minute, second;
    rtc_get_datetime(&year, &month, &day, &hour, &minute, &second);
    
    // Real uptime
    uint64_t up = uptime_ms();
    uint32_t up_sec = (uint32_t)(up / 1000);
    uint32_t up_min = up_sec / 60;
    up_sec = up_sec % 60;
    
    // Real PCI device count
    pci_device_t pci_devs[32];
    int pci_count = pci_enumerate_devices(pci_devs, 32);
    
    // Real framebuffer info
    extern uint32_t fb_width, fb_height;
    
    // Count VFS files
    int file_count = 0;
    for (int i = 0; i < 64; i++) {
        if (vfs_get_by_index(i)) file_count++;
    }
    
    // ASCII art logo + system info (like Linux fastfetch/neofetch)
    term_history_add("", 0x000000);
    term_history_add("       ____        __              ____  _____", 0x50E3C2);
    term_history_add("      / __ \\____  / /_____  ____  / __ \\/ ___/", 0x50E3C2);
    term_history_add("     / / / / __ \\/ //_/ _ \\/ __ \\/ / / /\\__ \\", 0x50E3C2);
    term_history_add("    / /_/ / / / / ,< /  __/ / / / /_/ /___/ /", 0x50E3C2);
    term_history_add("    \\____/_/ /_/_/|_|\\___/_/ /_/\\____//____/", 0x50E3C2);
    term_history_add("", 0x000000);
    
    char line[128];
    
    xsprintf(line, "  OS:         Onken OS v3.0 x86_64");
    term_history_add(line, 0xF5A623);
    
    xsprintf(line, "  Kernel:     oneko-kernel 3.0.0-limine");
    term_history_add(line, 0xFFFFFF);
    
    xsprintf(line, "  Host:       QEMU Virtual Machine");
    term_history_add(line, 0xFFFFFF);
    
    xsprintf(line, "  Uptime:     %d min %d sec", up_min, up_sec);
    term_history_add(line, 0xFFFFFF);
    
    xsprintf(line, "  Shell:      oneko-sh 1.0");
    term_history_add(line, 0xFFFFFF);
    
    xsprintf(line, "  Resolution: %dx%d @ 32bpp", fb_width, fb_height);
    term_history_add(line, 0xFFFFFF);
    
    xsprintf(line, "  Memory:     %d MB / %d MB (%d MB free)", used_mb, total_mb, free_mb);
    term_history_add(line, 0xFFFFFF);
    
    xsprintf(line, "  PCI:        %d devices detected", pci_count);
    term_history_add(line, 0xFFFFFF);
    
    xsprintf(line, "  VFS Files:  %d files in RAM disk", file_count);
    term_history_add(line, 0xFFFFFF);
    
    xsprintf(line, "  Date:       %d-%d-%d  %d:%d:%d UTC", year, month, day, hour, minute, second);
    term_history_add(line, 0xFFFFFF);
    
    xsprintf(line, "  Authors:    Ayham & Mason");
    term_history_add(line, 0xD4A017);
    
    term_history_add("", 0x000000);
    term_history_add("  Type 'help' for available commands.", 0x888888);
}

void terminal_init(void) {
    app_register(&terminal_app);
}

void terminal_launch(const char* args) {
    (void)args;
    history_count = 0;
    cmd_len = 0;
    memset(cmd_buf, 0, MAX_CMD);
    
    wm_add_window(200, 80, 750, 520, "Onken Terminal", terminal_draw);
    sys_create_task("terminal", 1);
    
    // Show real system fastfetch on launch!
    show_fastfetch();
}

static void print_to_shell(const char* msg, uint32_t color) {
    term_history_add(msg, color);
}

static void exec_command(window_t* w, const char* cmd) {
    char temp[MAX_CMD];
    strncpy(temp, cmd, MAX_CMD - 1);
    temp[MAX_CMD - 1] = '\0';
    char* argv0 = strtok(temp, " ");
    if (!argv0) return;

    // Check registered apps first
    app_entry_t* app = app_find(argv0);
    if (app && strcmp(argv0, "terminal") != 0) {
        char* args = strtok(NULL, "");
        if (args) {
            while (*args == ' ') args++;
            if (strlen(args) == 0) args = 0;
        }
        app->launch(args);
        char msg[128];
        if (args) {
            xsprintf(msg, "Launched %s with args: %s", app->name, args);
        } else {
            xsprintf(msg, "Launched %s.", app->name);
        }
        print_to_shell(msg, 0x00FF00);
        return;
    }

    // Builtin commands
    if (strcmp(argv0, "help") == 0) {
        print_to_shell("Commands:", 0x50E3C2);
        print_to_shell("  ls      - List VFS files", 0xFFFFFF);
        print_to_shell("  cat     - Show file contents", 0xFFFFFF);
        print_to_shell("  touch   - Create new file", 0xFFFFFF);
        print_to_shell("  rm      - Remove file", 0xFFFFFF);
        print_to_shell("  pci     - List PCI devices", 0xFFFFFF);
        print_to_shell("  date    - Show date/time", 0xFFFFFF);
        print_to_shell("  mem     - Memory info", 0xFFFFFF);
        print_to_shell("  ver     - OS version", 0xFFFFFF);
        print_to_shell("  clear   - Clear screen", 0xFFFFFF);
        print_to_shell("  fetch   - System info", 0xFFFFFF);
        print_to_shell("  exit    - Close terminal", 0xFFFFFF);
        
        print_to_shell("Applications:", 0x50E3C2);
        uint32_t cnt = app_get_count();
        for (uint32_t i = 0; i < cnt; i++) {
            app_entry_t* a = app_get_by_index(i);
            if (a && strcmp(a->name, "terminal") != 0) {
                char app_help[128];
                xsprintf(app_help, "  %-10s - %s", a->name, a->description);
                print_to_shell(app_help, 0xFFFFFF);
            }
        }
    } else if (strcmp(argv0, "ls") == 0) {
        print_to_shell("RAM Disk VFS:", 0xF5A623);
        int found = 0;
        for (int i = 0; i < 64; i++) {
            vfs_file_t* f = vfs_get_by_index(i);
            if (f) {
                char tmp[128];
                xsprintf(tmp, "  %-16s %5d bytes", f->name, (uint32_t)f->size);
                
                // Color by extension
                int nlen = strlen(f->name);
                uint32_t color = 0xFFFFFF;
                if (nlen >= 4 && strcmp(f->name + nlen - 4, ".bmp") == 0) color = 0xFF8800;
                else if (nlen >= 2 && strcmp(f->name + nlen - 2, ".c") == 0) color = 0x50E3C2;
                else if (nlen >= 4 && strcmp(f->name + nlen - 4, ".txt") == 0) color = 0x4A90E2;
                
                print_to_shell(tmp, color);
                found = 1;
            }
        }
        if (!found) print_to_shell("  (empty)", 0x888888);
        
    } else if (strcmp(argv0, "cat") == 0) {
        char* fname = strtok(NULL, " ");
        if (!fname) {
            print_to_shell("Usage: cat <filename>", 0xFFAAAA);
        } else {
            vfs_file_t* f = vfs_open(fname);
            if (!f) {
                print_to_shell("File not found.", 0xD0021B);
            } else {
                // Print file contents line by line
                char line_buf[128];
                int li = 0;
                for (size_t i = 0; i < f->size && i < 2048; i++) {
                    if (f->data[i] == '\n' || li >= 120) {
                        line_buf[li] = '\0';
                        print_to_shell(line_buf, 0xFFFFFF);
                        li = 0;
                    } else {
                        line_buf[li++] = f->data[i];
                    }
                }
                if (li > 0) {
                    line_buf[li] = '\0';
                    print_to_shell(line_buf, 0xFFFFFF);
                }
            }
        }
    } else if (strcmp(argv0, "touch") == 0) {
        char* fname = strtok(NULL, " ");
        if (!fname) {
            print_to_shell("Usage: touch <filename>", 0xFFAAAA);
        } else {
            if (vfs_create(fname) == 0) {
                char msg[64];
                xsprintf(msg, "Created %s", fname);
                print_to_shell(msg, 0x00FF00);
            } else {
                print_to_shell("Failed to create file.", 0xD0021B);
            }
        }
    } else if (strcmp(argv0, "rm") == 0) {
        char* fname = strtok(NULL, " ");
        if (!fname) {
            print_to_shell("Usage: rm <filename>", 0xFFAAAA);
        } else {
            if (vfs_remove(fname) == 0) {
                char msg[64];
                xsprintf(msg, "Removed %s", fname);
                print_to_shell(msg, 0x00FF00);
            } else {
                print_to_shell("File not found.", 0xD0021B);
            }
        }
    } else if (strcmp(argv0, "pci") == 0) {
        pci_device_t devs[32];
        int count = pci_enumerate_devices(devs, 32);
        char hdr[64];
        xsprintf(hdr, "PCI Devices: %d found", count);
        print_to_shell(hdr, 0xF5A623);
        for (int i = 0; i < count; i++) {
            char line[128];
            xsprintf(line, "  %d:%d.%d  %x:%x  class=%x sub=%x",
                     devs[i].bus, devs[i].device, devs[i].function,
                     devs[i].vendor_id, devs[i].device_id,
                     devs[i].class_code, devs[i].subclass);
            print_to_shell(line, 0xFFFFFF);
        }
    } else if (strcmp(argv0, "date") == 0) {
        int yr, mo, dy, hr, mi, se;
        rtc_get_datetime(&yr, &mo, &dy, &hr, &mi, &se);
        char line[64];
        xsprintf(line, "%d-%d-%d %d:%d:%d UTC", yr, mo, dy, hr, mi, se);
        print_to_shell(line, 0xFFFFFF);
    } else if (strcmp(argv0, "mem") == 0) {
        mem_info_t mem;
        sys_get_mem_info(&mem);
        char line[128];
        xsprintf(line, "Total: %d MB", (uint32_t)(mem.total_memory / 1024 / 1024));
        print_to_shell(line, 0xFFFFFF);
        xsprintf(line, "Used:  %d MB (%d allocs)", (uint32_t)(mem.used_memory / 1024 / 1024), (uint32_t)mem.heap_allocations);
        print_to_shell(line, 0xFFFFFF);
        xsprintf(line, "Free:  %d MB", (uint32_t)(mem.free_memory / 1024 / 1024));
        print_to_shell(line, 0x00FF00);
    } else if (strcmp(argv0, "fetch") == 0) {
        show_fastfetch();
    } else if (strcmp(argv0, "cd") == 0) {
        char* arg1 = strtok(NULL, " ");
        if (arg1) strncpy(current_dir, arg1, 63);
        else strncpy(current_dir, "/", 63);
    } else if (strcmp(argv0, "ver") == 0) {
        print_to_shell("Onken OS v3.0.0 (x86_64-limine)", 0xF5A623);
        print_to_shell("Built with GCC, freestanding C.", 0xFFFFFF);
    } else if (strcmp(argv0, "clear") == 0) {
        history_count = 0;
    } else if (strcmp(argv0, "exit") == 0) {
        w->closed = 1;
        sys_destroy_task(43);
    } else {
        char msg[128];
        xsprintf(msg, "'%s' is not recognized. Type 'help'.", argv0);
        print_to_shell(msg, 0xD0021B);
    }
}

void terminal_draw(void* self) {
    window_t* w = (window_t*)self;
    
    // Fill dark console background
    fb_rect(w->x + 6, w->y + 26, w->w - 12, w->h - 32, 0x111111);
    
    uint32_t sy = w->y + 30;
    int max_visible = (w->h - 60) / 12;
    int start = history_count > max_visible ? history_count - max_visible : 0;
    
    for (int i = start; i < history_count; i++) {
        if (sy + 12 > w->y + w->h - 30) break;
        fb_print(term_history[i].text, w->x + 10, sy, term_history[i].color, 0x111111);
        sy += 12;
    }
    
    // Draw prompt
    fb_print("root@onken:", w->x + 10, sy, 0x50E3C2, 0x111111);
    fb_print(current_dir, w->x + 10 + 11*8, sy, 0x4A90E2, 0x111111);
    fb_print("$ ", w->x + 10 + (11 + strlen(current_dir))*8, sy, 0xBD10E0, 0x111111);
    fb_print(cmd_buf, w->x + 10 + (13 + strlen(current_dir))*8, sy, 0xFFFFFF, 0x111111);
}

void terminal_handle_key(char c) {
    window_t* active_w = wm_get_active();
    if (!active_w || active_w->closed) return;

    if (c == '\n' || c == '\r') {
        cmd_buf[cmd_len] = '\0';
        char full_cmd[128];
        xsprintf(full_cmd, "root@onken:%s$ %s", current_dir, cmd_buf);
        term_history_add(full_cmd, 0x888888);
        
        exec_command(active_w, cmd_buf);
        cmd_len = 0;
        memset(cmd_buf, 0, MAX_CMD);
    } else if (c == '\b') {
        if (cmd_len > 0) cmd_buf[--cmd_len] = '\0';
    } else if (c >= 32 && c < 127) {
        if (cmd_len < MAX_CMD - 1) cmd_buf[cmd_len++] = c;
    }
}
