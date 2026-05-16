#include "yano.h"
#include "../../kernel/tasking/app.h"
#include "../../kernel/syscalls/syscall.h"
#include "../../gui/fb.h"
#include "../../gui/window.h"
#include "../../kernel/string.h"
#include "../../kernel/fs.h"

static char yano_buf[1024];
static uint32_t yano_len = 0;
static char current_filename[32] = "new.txt";
static char window_title[64] = "yano - new.txt";

static app_entry_t yano_app = {
    .name = "yano",
    .title = "yano", // Base match
    .description = "Text Editor",
    .init = yano_init,
    .launch = yano_launch,
    .draw = yano_draw,
    .handle_key = yano_handle_key
};

void yano_init(void) {
    app_register(&yano_app);
}

void yano_launch(const char* args) {
    if (args && strlen(args) > 0) {
        strncpy(current_filename, args, 31);
        current_filename[31] = '\0';
        
        // Open file
        vfs_file_t* file = vfs_open(current_filename);
        if (file) {
            strncpy(yano_buf, file->data, 1023);
            yano_buf[1023] = '\0';
            yano_len = file->size;
        } else {
            // Create new file in VFS
            vfs_create(current_filename);
            yano_buf[0] = '\0';
            yano_len = 0;
        }
    } else {
        strncpy(current_filename, "new.txt", 31);
        strncpy(yano_buf, "Welcome to yano!\nStart typing here...\nCtrl+S to Save, Ctrl+X to Exit.", 1023);
        yano_len = strlen(yano_buf);
    }
    
    xsprintf(window_title, "yano - %s", current_filename);
    wm_add_window(180, 100, 600, 400, window_title, yano_draw);
    sys_create_task("yano", 1);
}

void yano_draw(void* self) {
    window_t* w = (window_t*)self;
    
    // Yano header
    fb_rect(w->x + 2, w->y + 22, w->w - 4, 16, 0xDDDDDD);
    char header[64];
    xsprintf(header, "  UW PICO 5.09               File: %s", current_filename);
    fb_print(header, w->x + 10, w->y + 26, 0x000000, 0xDDDDDD);
    
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

void yano_handle_key(char c) {
    int ctrl = sys_is_ctrl_pressed();
    
    if (ctrl) {
        if (c == 'x' || c == 'X') {
            // Exit yano
            window_t* active_w = wm_get_active();
            if (active_w) {
                active_w->closed = 1;
                sys_print("yano closed.", 0xFFFFFF);
            }
            return;
        } else if (c == 's' || c == 'S' || c == 'o' || c == 'O') {
            // Save file
            vfs_write(current_filename, yano_buf, yano_len);
            
            char msg[64];
            xsprintf(msg, "Saved %d bytes to VFS:%s", yano_len, current_filename);
            sys_print(msg, 0x00FF00);
            return;
        }
        return; // Discard other ctrl combos
    }

    // Standard Typing
    if (c == '\n' || c == '\r') {
        if (yano_len < 1023) {
            yano_buf[yano_len++] = '\n';
            yano_buf[yano_len] = '\0';
        }
    } else if (c == '\b') {
        if (yano_len > 0) {
            yano_buf[--yano_len] = '\0';
        }
    } else if (c >= 32 && c < 127) {
        if (yano_len < 1023) {
            yano_buf[yano_len++] = c;
            yano_buf[yano_len] = '\0';
        }
    }
}
