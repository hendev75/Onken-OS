#include "files.h"
#include "../../kernel/tasking/app.h"
#include "../../kernel/syscalls/syscall.h"
#include "../../gui/fb.h"
#include "../../gui/window.h"
#include "../../kernel/string.h"

static app_entry_t files_app = {
    .name = "files",
    .title = "File Explorer",
    .description = "File Manager",
    .init = files_init,
    .launch = files_launch,
    .draw = files_draw,
    .handle_key = 0
};

void files_init(void) {
    app_register(&files_app);
}

void files_launch(const char* args) {
    (void)args;
    wm_add_window(200, 150, 600, 400, "File Explorer", files_draw);
    sys_create_task("files", 1);
}

void files_draw(void* self) {
    window_t* w = (window_t*)self;
    
    fb_print("System Drive (C:)", w->x + 20, w->y + 40, 0x882200, 0xC0C0C0);
    
    fb_print("[DIR]  boot/", w->x + 20, w->y + 70, 0x0055AA, 0xC0C0C0);
    fb_print("[DIR]  drivers/", w->x + 20, w->y + 90, 0x0055AA, 0xC0C0C0);
    fb_print("[DIR]  gui/", w->x + 20, w->y + 110, 0x0055AA, 0xC0C0C0);
    fb_print("[DIR]  kernel/", w->x + 20, w->y + 130, 0x0055AA, 0xC0C0C0);
    fb_print("[DIR]  shell/", w->x + 20, w->y + 150, 0x0055AA, 0xC0C0C0);
    
    fb_print("[FILE] Makefile", w->x + 240, w->y + 70, 0x222222, 0xC0C0C0);
    fb_print("[FILE] limine.conf", w->x + 240, w->y + 90, 0x222222, 0xC0C0C0);
    fb_print("[FILE] readme.txt", w->x + 240, w->y + 110, 0x222222, 0xC0C0C0);
}
