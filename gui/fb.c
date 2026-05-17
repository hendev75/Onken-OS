#include "fb.h"
#include "font.h"
#include "../drivers/ps2.h"
#include "../kernel/string.h"
#include "window.h"

static uint32_t* fb_ptr;
uint32_t fb_width, fb_height, fb_pitch;
static uint8_t fb_bpp;
static uint32_t* back_buffer = 0;
static uint32_t* wallpaper_cache = 0;
static uint32_t* desktop_buffer = 0;
static uint32_t* front_buffer_copy = 0;

// Optimized 32-bit memcpy to prevent pixel dropping
static void memcpy32(void* dest, const void* src, uint32_t size) {
    uint32_t* d = (uint32_t*)dest;
    const uint32_t* s = (const uint32_t*)src;
    uint32_t count = size / 4;
    while (count--) *d++ = *s++;
}

// Mouse cursor state
#define CURSOR_W 12
#define CURSOR_H 18

void fb_init_raw(void* addr, uint32_t w, uint32_t h, uint32_t pitch) {
    fb_ptr = (uint32_t*)addr;
    fb_width = w;
    fb_height = h;
    fb_pitch = pitch;
}

void fb_init(multiboot_tag_framebuffer_t* fb_tag) {
    fb_ptr = (uint32_t*)(uint64_t)fb_tag->framebuffer_addr;
    fb_width = fb_tag->framebuffer_width;
    fb_height = fb_tag->framebuffer_height;
    fb_pitch = fb_tag->framebuffer_pitch / 4; // pitch in pixels
    fb_bpp = fb_tag->framebuffer_bpp;
}

void fb_alloc_buffers() {
    uint32_t size = fb_width * fb_height * 4;
    back_buffer = (uint32_t*)kmalloc(size);
    wallpaper_cache = (uint32_t*)kmalloc(size);
    desktop_buffer = (uint32_t*)kmalloc(size);
    front_buffer_copy = (uint32_t*)kmalloc(size);
    
    // Clear front buffer to ensure first frame is drawn entirely
    memset(front_buffer_copy, 0, size);
}

void fb_swap() {
    // Diff-based swap: Only copy changed contiguous pixel blocks
    for (uint32_t y = 0; y < fb_height; y++) {
        uint32_t row_offset = y * (fb_pitch / 4);
        uint32_t first_diff = fb_width;
        uint32_t last_diff = 0;
        
        for (uint32_t x = 0; x < fb_width; x++) {
            if (back_buffer[row_offset + x] != front_buffer_copy[row_offset + x]) {
                if (first_diff == fb_width) first_diff = x;
                last_diff = x;
            }
        }
        
        if (first_diff < fb_width) {
            uint32_t count = last_diff - first_diff + 1;
            uint32_t start_idx = row_offset + first_diff;
            
            // Copy to VGA
            memcpy32(fb_ptr + start_idx, back_buffer + start_idx, count * 4);
            // Update RAM copy
            memcpy32(front_buffer_copy + start_idx, back_buffer + start_idx, count * 4);
        }
    }
}

void fb_commit_desktop() {
    memcpy32(desktop_buffer, back_buffer, fb_width * fb_height * 4);
}

void fb_restore_desktop_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    if (x >= fb_width || y >= fb_height) return;
    if (x + w > fb_width) w = fb_width - x;
    if (y + h > fb_height) h = fb_height - y;
    
    for (uint32_t iy = y; iy < y + h; iy++) {
        uint32_t offset = (iy * (fb_pitch / 4)) + x;
        memcpy32(back_buffer + offset, desktop_buffer + offset, w * 4);
    }
}

void fb_clear_to_wallpaper() {
    memcpy32(back_buffer, wallpaper_cache, fb_width * fb_height * 4);
}

extern int wallpaper_mode;

void fb_render_wallpaper() {
    uint32_t* old_back = back_buffer;
    back_buffer = wallpaper_cache;
    
    if (wallpaper_mode == 1) {
        // Retro theme: Solid Slate Blue (Amiga Workbench look!)
        fb_rect(0, 0, fb_width, fb_height, 0x3B6790);
    } else if (wallpaper_mode == 0) {
        // BoredOS style gradient: deep dark blue to black
        fb_gradient(0, 0, fb_width, fb_height, 0x1A1A2E, 0x16213E);
        
        // Add some subtle "stars" or details
        for(uint32_t i = 0; i < 200; i++) {
            uint32_t rx = (uint32_t)(123456789ULL * (i+1) % fb_width); // fake rand
            uint32_t ry = (uint32_t)(987654321ULL * (i+1) % fb_height);
            fb_plot(rx, ry, 0x444466);
        }
    } else {
        // Default solid color
        fb_rect(0, 0, fb_width, fb_height, 0x111111);
    }

    back_buffer = old_back;
}

void fb_clear(uint32_t color) {
    for(uint32_t y = 0; y < fb_height; y++) {
        for(uint32_t x = 0; x < fb_width; x++) {
            back_buffer[y * (fb_pitch / 4) + x] = color;
        }
    }
}

void fb_gradient(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t c1, uint32_t c2) {
    for (uint32_t j = 0; j < h; j++) {
        uint32_t r1 = (c1 >> 16) & 0xFF;
        uint32_t g1 = (c1 >> 8) & 0xFF;
        uint32_t b1 = c1 & 0xFF;
        uint32_t r2 = (c2 >> 16) & 0xFF;
        uint32_t g2 = (c2 >> 8) & 0xFF;
        uint32_t b2 = c2 & 0xFF;

        uint32_t r = r1 + (int32_t)(r2 - r1) * (int32_t)j / (int32_t)h;
        uint32_t g = g1 + (int32_t)(g2 - g1) * (int32_t)j / (int32_t)h;
        uint32_t b = b1 + (int32_t)(b2 - b1) * (int32_t)j / (int32_t)h;
        uint32_t color = (r << 16) | (g << 8) | b;

        for (uint32_t i = 0; i < w; i++) {
            fb_plot(x + i, y + j, color);
        }
    }
}

#include "../kernel/fs.h"

void draw_bmp_to_fb(const char* data, size_t size, int screen_x, int screen_y, int max_w, int max_h) {
    if (size < 54) return;
    if (data[0] != 'B' || data[1] != 'M') return;
    
    int w, h;
    // Safely copy width and height from header offsets (avoid unaligned access crash)
    w = *(const int32_t*)(data + 18);
    h = *(const int32_t*)(data + 22);
    uint16_t bpp = *(const uint16_t*)(data + 28);
    uint32_t offset = *(const uint32_t*)(data + 10);
    
    if (bpp != 24) return; // Support standard 24bpp uncompressed
    
    size_t data_idx = offset;
    for (int y = h - 1; y >= 0; y--) {
        for (int x = 0; x < w; x++) {
            if (data_idx + 3 > size) break;
            uint8_t b = data[data_idx++];
            uint8_t g = data[data_idx++];
            uint8_t r = data[data_idx++];
            
            uint32_t color = (r << 16) | (g << 8) | b;
            
            if (x < max_w && (h - 1 - y) < max_h) {
                fb_plot(screen_x + x, screen_y + (h - 1 - y), color);
            }
        }
    }
}

void fb_wallpaper() {
    vfs_file_t* f = vfs_open("sunset.bmp");
    if (f) {
        int w = *(const int32_t*)(f->data + 18);
        int h = *(const int32_t*)(f->data + 22);
        if (w <= 0 || h <= 0) { w = 64; h = 64; }
        
        for (uint32_t ty = 0; ty < fb_height; ty += h) {
            for (uint32_t tx = 0; tx < fb_width; tx += w) {
                draw_bmp_to_fb(f->data, f->size, tx, ty, w, h);
            }
        }
    } else {
        fb_gradient(0, 0, fb_width, fb_height, 0x1a2a6c, 0x0a0a0a);
    }
}

void fb_plot(uint32_t x, uint32_t y, uint32_t color) {
    if (x < fb_width && y < fb_height)
        back_buffer[y * (fb_pitch / 4) + x] = color;
}

uint32_t fb_get_pixel(uint32_t x, uint32_t y) {
    if (x < fb_width && y < fb_height)
        return back_buffer[y * (fb_pitch / 4) + x];
    return 0;
}

void fb_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    if (x >= fb_width || y >= fb_height) return;
    if (x + w > fb_width) w = fb_width - x;
    if (y + h > fb_height) h = fb_height - y;
    
    for (uint32_t iy = y; iy < y + h; iy++) {
        uint32_t offset = iy * (fb_pitch / 4) + x;
        for (uint32_t ix = 0; ix < w; ix++) {
            back_buffer[offset + ix] = color;
        }
    }
}

void fb_putc(char c, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg) {
    uint8_t u = (uint8_t)c;
    if (u >= 128) u = '?'; // fallback for non-ASCII
    const uint8_t* glyph = font8x8[u];
    
    for (uint32_t row = 0; row < 8; row++) {
        for (uint32_t col = 0; col < 8; col++) {
            if (glyph[row] & (1 << (7 - col))) {
                fb_plot(x + col, y + row, fg);
            } else {
                // If bg is distinct from transparent (e.g. we use MSB for transparency later)
                // For now, let's just always draw the background to overwrite old text.
                fb_plot(x + col, y + row, bg);
            }
        }
    }
}

void fb_print(const char* str, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg) {
    uint32_t cx = x;
    uint32_t cy = y;
    while (*str) {
        if (*str == '\n') {
            cx = x;
            cy += 10;
        } else {
            fb_putc(*str, cx, cy, fg, bg);
            cx += 8;
        }
        str++;
    }
}

static uint32_t cursor_backup[32 * 32];
static int32_t backup_x = -1, backup_y = -1;
static uint32_t backup_w = 0, backup_h = 0;

void fb_reset_cursor_backup() {
    backup_x = -1;
}

void fb_hide_cursor() {
    if (backup_x == -1) return;
    for (uint32_t y = 0; y < backup_h; y++) {
        uint32_t dest_y = backup_y + y;
        uint32_t row_offset = dest_y * (fb_pitch / 4);
        for (uint32_t x = 0; x < backup_w; x++) {
            uint32_t dest_x = backup_x + x;
            back_buffer[row_offset + dest_x] = cursor_backup[y * 32 + x];
        }
    }
    backup_x = -1;
}

void fb_show_cursor(int32_t x, int32_t y) {
    fb_hide_cursor();
    
    backup_x = x;
    backup_y = y;
    backup_w = 32;
    backup_h = 32;
    
    if (backup_x < 0) backup_x = 0;
    if (backup_y < 0) backup_y = 0;
    if (backup_x + (int)backup_w > (int)fb_width) backup_w = fb_width - backup_x;
    if (backup_y + (int)backup_h > (int)fb_height) backup_h = fb_height - backup_y;
    
    for (uint32_t cy = 0; cy < backup_h; cy++) {
        uint32_t src_y = backup_y + cy;
        uint32_t row_offset = src_y * (fb_pitch / 4);
        for (uint32_t cx = 0; cx < backup_w; cx++) {
            uint32_t src_x = backup_x + cx;
            cursor_backup[cy * 32 + cx] = back_buffer[row_offset + src_x];
        }
    }
    
    for(int cy = 0; cy < 12; cy++) {
        for(int cx = 0; cx <= cy; cx++) {
            if (x + cx < (int)fb_width && y + cy < (int)fb_height) {
                if (cx == 0 || cy == 11 || cx == cy) {
                    fb_plot(x + cx, y + cy, 0x000000);
                } else {
                    fb_plot(x + cx, y + cy, 0xFFFFFF);
                }
            }
        }
    }
    if (x + 10 < (int)fb_width && y + 15 < (int)fb_height) {
        fb_plot(x+5, y+12, 0x000000); fb_plot(x+6, y+12, 0x000000); fb_plot(x+7, y+12, 0x000000);
        fb_plot(x+6, y+13, 0xFFFFFF); fb_plot(x+7, y+13, 0xFFFFFF); fb_plot(x+8, y+13, 0x000000);
        fb_plot(x+7, y+14, 0xFFFFFF); fb_plot(x+8, y+14, 0xFFFFFF); fb_plot(x+9, y+14, 0x000000);
        fb_plot(x+8, y+15, 0x000000); fb_plot(x+9, y+15, 0x000000); fb_plot(x+10, y+15, 0x000000);
    }
}

// STB Image integration
extern unsigned char *stbi_load_from_memory(unsigned char const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels);
extern void stbi_image_free(void *retval_from_stbi_load);

void fb_draw_image(const char* data, size_t size, int screen_x, int screen_y, int max_w, int max_h) {
    int w, h, channels;
    unsigned char *img = stbi_load_from_memory((const unsigned char*)data, size, &w, &h, &channels, 4);
    if (!img) {
        // Fallback or error text
        fb_print("Unsupported image format.", screen_x + 10, screen_y + 10, 0xFF5555, 0x222222);
        return;
    }

    // Scale to fit max_w, max_h while maintaining aspect ratio
    int draw_w = w;
    int draw_h = h;

    if (w > max_w || h > max_h) {
        float scale_w = (float)max_w / w;
        float scale_h = (float)max_h / h;
        float scale = (scale_w < scale_h) ? scale_w : scale_h;
        draw_w = (int)(w * scale);
        draw_h = (int)(h * scale);
    }
    
    // Center the image
    int off_x = screen_x + (max_w - draw_w) / 2;
    int off_y = screen_y + (max_h - draw_h) / 2;

    // Nearest neighbor scaling render
    for (int y = 0; y < draw_h; y++) {
        int src_y = (y * h) / draw_h;
        for (int x = 0; x < draw_w; x++) {
            int src_x = (x * w) / draw_w;
            int src_idx = (src_y * w + src_x) * 4;
            
            uint8_t r = img[src_idx + 0];
            uint8_t g = img[src_idx + 1];
            uint8_t b = img[src_idx + 2];
            uint8_t a = img[src_idx + 3];
            
            if (a > 128) {
                fb_plot(off_x + x, off_y + y, (r << 16) | (g << 8) | b);
            }
        }
    }

    // Since our memory allocator doesn't really free, we leak the decoded image buffer.
    // In a real OS with a real allocator, we would call stbi_image_free(img).
    // For this prototype, we'll let it bump allocate (reloading images will eat RAM).
    // stbi_image_free(img);
}