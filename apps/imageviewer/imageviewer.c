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
    wm_add_window(150, 100, 320, 240, "Image Viewer", imageviewer_draw);
    sys_create_task("imageviewer", 1);
}

void imageviewer_draw(void* self) {
    window_t* w = (window_t*)self;
    
    // Header label
    fb_print("Image: ", w->x + 20, w->y + 40, 0x882200, 0xC0C0C0);
    fb_print(current_image, w->x + 80, w->y + 40, 0x000000, 0xC0C0C0);
    
    // Draw 3D sunken viewport for image
    draw_retro_3d_panel(w->x + 20, w->y + 60, w->w - 40, w->h - 80, 1);
    fb_rect(w->x + 22, w->y + 62, w->w - 44, w->h - 84, 0x111111); // black background
    
    vfs_file_t* f = vfs_open(current_image);
    if (f) {
        // Draw the decoded BMP image inside the black panel!
        extern void draw_bmp_to_fb(const char* data, size_t size, int screen_x, int screen_y, int max_w, int max_h);
        draw_bmp_to_fb(f->data, f->size, w->x + 22, w->y + 62, w->w - 44, w->h - 84);
    } else {
        fb_print("Image file not found.", w->x + 30, w->y + 70, 0xFFFFFF, 0x111111);
    }
}
