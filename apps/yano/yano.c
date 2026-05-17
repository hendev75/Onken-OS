#include "yano.h"
#include "../../kernel/tasking/app.h"
#include "../../kernel/syscalls/syscall.h"
#include "../../gui/fb.h"
#include "../../gui/window.h"
#include "../../kernel/string.h"
#include "../../kernel/fs.h"

#define YANO_BUF_SIZE 4096
static char yano_buf[YANO_BUF_SIZE];
static uint32_t yano_len = 0;
static char current_filename[32] = "new.txt";
static char window_title[64] = "yano - new.txt";

static app_entry_t yano_app = {
    .name = "yano",
    .title = "yano",
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
    // Reset buffer
    memset(yano_buf, 0, YANO_BUF_SIZE);
    yano_len = 0;
    
    if (args && strlen(args) > 0) {
        strncpy(current_filename, args, 31);
        current_filename[31] = '\0';
        
        // Try to open file from VFS
        vfs_file_t* file = vfs_open(current_filename);
        if (file && file->size > 0) {
            size_t copy_len = file->size;
            if (copy_len > YANO_BUF_SIZE - 1) copy_len = YANO_BUF_SIZE - 1;
            memcpy(yano_buf, file->data, copy_len);
            yano_buf[copy_len] = '\0';
            yano_len = (uint32_t)copy_len;
        } else if (!file) {
            // Create new file in VFS
            vfs_create(current_filename);
            yano_buf[0] = '\0';
            yano_len = 0;
        }
    } else {
        strncpy(current_filename, "new.txt", 31);
        const char* welcome = "Welcome to yano!\nStart typing here...\nCtrl+S to Save, Ctrl+X to Exit.";
        strncpy(yano_buf, welcome, YANO_BUF_SIZE - 1);
        yano_len = strlen(yano_buf);
    }
    
    xsprintf(window_title, "yano - %s", current_filename);
    wm_add_window(180, 100, 600, 400, window_title, yano_draw);
    sys_create_task("yano", 1);
}

void yano_draw(void* self) {
    window_t* w = (window_t*)self;
    
    // Dark editor background
    fb_rect(w->x + 6, w->y + 26, w->w - 12, w->h - 32, 0x111111);
    
    // Header bar
    fb_rect(w->x + 6, w->y + 26, w->w - 12, 16, 0xDDDDDD);
    char header[80];
    xsprintf(header, "  YANO Editor  |  File: %s  |  %d bytes", current_filename, yano_len);
    fb_print(header, w->x + 12, w->y + 30, 0x000000, 0xDDDDDD);
    
    // Content area
    int cx = 0, cy = 0;
    int max_y = (int)w->h - 72;
    
    for (uint32_t i = 0; i < yano_len; i++) {
        if (yano_buf[i] == '\n') {
            cy += 14;
            cx = 0;
        } else {
            if (cy < max_y) {
                char temp[2] = {yano_buf[i], '\0'};
                fb_print(temp, w->x + 12 + cx, w->y + 48 + cy, 0xFFFFFF, 0x111111);
            }
            cx += 8;
            if (cx > (int)w->w - 30) {
                cx = 0;
                cy += 14;
            }
        }
    }

    // Cursor
    if (cy < max_y) {
        fb_rect(w->x + 12 + cx, w->y + 48 + cy, 8, 12, 0xFFFFFF);
    }

    // Footer shortcuts bar
    fb_rect(w->x + 6, w->y + w->h - 36, w->w - 12, 30, 0xDDDDDD);
    fb_print("^S Save  ^X Exit  ^O WriteOut  Backspace=Delete", w->x + 12, w->y + w->h - 32, 0x000000, 0xDDDDDD);
    fb_print("Type normally. Changes auto-tracked.", w->x + 12, w->y + w->h - 20, 0x555555, 0xDDDDDD);
}

void yano_handle_key(char c) {
    int ctrl = sys_is_ctrl_pressed();
    
    if (ctrl) {
        if (c == 'x' || c == 'X') {
            window_t* active_w = wm_get_active();
            if (active_w) {
                active_w->closed = 1;
            }
            return;
        } else if (c == 's' || c == 'S' || c == 'o' || c == 'O') {
            // Save file to VFS
            vfs_file_t* f = vfs_open(current_filename);
            if (!f) {
                vfs_create(current_filename);
            }
            vfs_write(current_filename, yano_buf, yano_len);
            return;
        }
        return;
    }

    // Standard typing
    if (c == '\n' || c == '\r') {
        if (yano_len < YANO_BUF_SIZE - 1) {
            yano_buf[yano_len++] = '\n';
            yano_buf[yano_len] = '\0';
        }
    } else if (c == '\b') {
        if (yano_len > 0) {
            yano_buf[--yano_len] = '\0';
        }
    } else if (c >= 32 && c < 127) {
        if (yano_len < YANO_BUF_SIZE - 1) {
            yano_buf[yano_len++] = c;
            yano_buf[yano_len] = '\0';
        }
    }
}
