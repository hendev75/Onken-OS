#include "files.h"
#include "../../kernel/tasking/app.h"
#include "../../kernel/syscalls/syscall.h"
#include "../../gui/fb.h"
#include "../../gui/window.h"
#include "../../kernel/string.h"
#include "../../kernel/fs.h"

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
    
    fb_print("System Drive (RAM Disk)", w->x + 20, w->y + 40, 0x882200, 0xC0C0C0);
    
    // Draw columns
    fb_print("Directories:", w->x + 20, w->y + 70, 0x882200, 0xC0C0C0);
    fb_print("[DIR]  system/", w->x + 20, w->y + 90, 0x0055AA, 0xC0C0C0);
    fb_print("[DIR]  apps/", w->x + 20, w->y + 110, 0x0055AA, 0xC0C0C0);
    fb_print("[DIR]  assets/", w->x + 20, w->y + 130, 0x0055AA, 0xC0C0C0);
    
    fb_print("Files:", w->x + 260, w->y + 70, 0x882200, 0xC0C0C0);
    
    // Query VFS dynamically
    extern vfs_file_t* vfs_get_by_index(int idx);
    uint32_t sy = w->y + 90;
    for (int i = 0; i < 64; i++) {
        vfs_file_t* f = vfs_get_by_index(i);
        if (f) {
            char file_lbl[64];
            xsprintf(file_lbl, "[FILE] %-12s (%3d B)", f->name, (uint32_t)f->size);
            fb_print(file_lbl, w->x + 260, sy, 0x222222, 0xC0C0C0);
            sy += 20;
        }
    }
}
