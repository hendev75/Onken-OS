#include "yano.h"
#include "../../kernel/tasking/app.h"
#include "../../kernel/syscalls/syscall.h"
#include "../../gui/fb.h"
#include "../../gui/window.h"
#include "../../kernel/string.h"

static char yano_buf[1024] = "This is yano (yet another nano).\nThe ultra-fast text editor for Onken OS.\nStart typing here...";
static uint32_t yano_len = 86;

static app_entry_t yano_app = {
    .name = "yano",
    .title = "yano - file.txt",
    .description = "Text Editor",
    .init = yano_init,
    .launch = yano_launch,
    .draw = yano_draw,
    .handle_key = yano_handle_key
};

void yano_init(void) {
    app_register(&yano_app);
}

void yano_launch(void) {
    wm_add_window(200, 200, 600, 400, "yano - file.txt", yano_draw);
    sys_create_task("yano", 1);
}

void yano_draw(void* self) {
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

void yano_handle_key(char c) {
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
