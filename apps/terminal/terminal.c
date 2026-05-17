#include "terminal.h"
#include "../../kernel/tasking/app.h"
#include "../../kernel/syscalls/syscall.h"
#include "../../gui/fb.h"
#include "../../gui/window.h"
#include "../../kernel/string.h"

#define MAX_CMD 128
static char cmd_buf[MAX_CMD];
static uint32_t cmd_len = 0;
static char current_dir[64] = "/";

#define MAX_HISTORY 20
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

void terminal_init(void) {
    app_register(&terminal_app);
}

void terminal_launch(const char* args) {
    (void)args;
    wm_add_window(300, 200, 700, 500, "Onken Terminal", terminal_draw);
    sys_create_task("terminal", 1);
}

static void print_to_shell(const char* msg, uint32_t color) {
    term_history_add(msg, color);
}

static void exec_command(window_t* w, const char* cmd) {
    char temp[MAX_CMD];
    strncpy(temp, cmd, MAX_CMD);
    char* argv0 = strtok(temp, " ");
    if (!argv0) return;

    // Check if it is a registered application first (dynamic dispatch)
    app_entry_t* app = app_find(argv0);
    if (app) {
        char* args = strtok(NULL, "");
        if (args) {
            while (*args == ' ') args++; // Trim leading spaces
            if (strlen(args) == 0) args = 0;
        }
        app->launch(args);
        char msg[128];
        if (args) {
            xsprintf(msg, "Launched %s with args: %s", app->name, args);
        } else {
            xsprintf(msg, "Launched %s application.", app->name);
        }
        print_to_shell(msg, 0x00FF00);
        return;
    }

    // Builtin Commands
    if (strcmp(argv0, "help") == 0) {
        print_to_shell("Available Commands:", 0x50E3C2);
        print_to_shell("  cd    - Change directory", 0xFFFFFF);
        print_to_shell("  ls    - List directory", 0xFFFFFF);
        
        // Print all dynamically registered applications
        uint32_t cnt = app_get_count();
        for (uint32_t i = 0; i < cnt; i++) {
            app_entry_t* a = app_get_by_index(i);
            if (strcmp(a->name, "terminal") != 0) {
                char app_help[128];
                xsprintf(app_help, "  %-5s - %s", a->name, a->description);
                print_to_shell(app_help, 0xFFFFFF);
            }
        }
        
        print_to_shell("  ver   - OS version", 0xFFFFFF);
        print_to_shell("  clear - Clear screen", 0xFFFFFF);
        print_to_shell("  exit  - Close terminal", 0xFFFFFF);
    } else if (strcmp(argv0, "ls") == 0) {
        if (strcmp(current_dir, "/") == 0) {
            print_to_shell("boot/     drivers/  gui/      kernel/", 0x4A90E2);
            print_to_shell("shell/    Makefile  readme    onken.bin", 0xFFFFFF);
        } else {
            print_to_shell("file1.txt  file2.txt", 0xFFFFFF);
        }
    } else if (strcmp(argv0, "cd") == 0) {
        char* arg1 = strtok(NULL, " ");
        if (arg1) {
            strncpy(current_dir, arg1, 63);
        } else {
            strncpy(current_dir, "/", 63);
        }
    } else if (strcmp(argv0, "ver") == 0) {
        print_to_shell("Onken OS Revolution v3.0.0", 0xF5A623);
    } else if (strcmp(argv0, "clear") == 0) {
        history_count = 0;
    } else if (strcmp(argv0, "exit") == 0) {
        w->closed = 1;
        // In a real OS we would kill the terminal task
        sys_destroy_task(43); // terminal base PID
    } else {
        print_to_shell("Command not found.", 0xD0021B);
    }
}

void terminal_draw(void* self) {
    window_t* w = (window_t*)self;
    
    // Fill the inner sunken panel with a solid dark grey console background
    fb_rect(w->x + 6, w->y + 26, w->w - 12, w->h - 32, 0x111111);
    
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

void terminal_handle_key(char c) {
    window_t* active_w = wm_get_active();
    if (!active_w || active_w->closed) return;

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
}
