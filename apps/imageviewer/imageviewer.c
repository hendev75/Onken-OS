#include "imageviewer.h"
#include "../../kernel/tasking/app.h"
#include "../../kernel/syscalls/syscall.h"
#include "../../gui/fb.h"
#include "../../gui/window.h"
#include "../../kernel/string.h"
#include "../../kernel/fs.h"

static char current_image[32] = "sunset.bmp";

static app_entry_t imageviewer_app = {
    .name = "imageviewer",
    .title = "Image Viewer",
    .description = "BMP Image Decoder",
    .init = imageviewer_init,
    .launch = imageviewer_launch,
    .draw = imageviewer_draw,
    .handle_key = 0
};

void imageviewer_init(void) {
    app_register(&imageviewer_app);
}

void imageviewer_launch(const char* args) {
    if (args && strlen(args) > 0) {
        strncpy(current_image, args, 31);
        current_image[31] = '\0';
    } else {
        strncpy(current_image, "sunset.bmp", 31);
    }
    wm_add_window(150, 100, 400, 340, "Image Viewer", imageviewer_draw);
    sys_create_task("imageviewer", 1);
}

void imageviewer_draw(void* self) {
    window_t* w = (window_t*)self;
    
    vfs_file_t* f = vfs_open(current_image);
    
    // Header with real file info
    if (f && f->size >= 54 && f->data[0] == 'B' && f->data[1] == 'M') {
        int img_w = *(const int32_t*)(f->data + 18);
        int img_h = *(const int32_t*)(f->data + 22);
        uint16_t bpp = *(const uint16_t*)(f->data + 28);
        
        char info[80];
        xsprintf(info, "%s  %dx%d  %dbpp  %d bytes", current_image, img_w, img_h, bpp, (uint32_t)f->size);
        fb_print(info, w->x + 12, w->y + 40, 0x000000, 0xC0C0C0);
    } else {
        fb_print("Image: ", w->x + 12, w->y + 40, 0x882200, 0xC0C0C0);
        fb_print(current_image, w->x + 72, w->y + 40, 0x000000, 0xC0C0C0);
    }
    
    // 3D sunken viewport
    int vx = w->x + 10;
    int vy = w->y + 56;
    int vw = w->w - 20;
    int vh = w->h - 70;
    draw_retro_3d_panel(vx, vy, vw, vh, 1);
    fb_rect(vx + 2, vy + 2, vw - 4, vh - 4, 0x222222);
    
    if (f && f->size >= 54 && f->data[0] == 'B' && f->data[1] == 'M') {
        // Use the proper draw_bmp_to_fb from fb.h
        draw_bmp_to_fb(f->data, f->size, vx + 4, vy + 4, vw - 8, vh - 8);
    } else {
        fb_print("File not found or invalid BMP.", vx + 10, vy + 20, 0xFF5555, 0x222222);
        fb_print("Use 'ls' to see available files.", vx + 10, vy + 40, 0xAAAAAA, 0x222222);
    }
}
