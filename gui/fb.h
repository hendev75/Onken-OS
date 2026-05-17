#pragma once
#include <stdint.h>
#include "../kernel/kernel.h"

extern uint32_t fb_width, fb_height;

void fb_init_raw(void* addr, uint32_t w, uint32_t h, uint32_t pitch);
void fb_init(multiboot_tag_framebuffer_t* fb_tag);
void fb_alloc_buffers();
void fb_swap();
void fb_clear_to_wallpaper();
void fb_render_wallpaper();
void fb_commit_desktop();
void fb_restore_desktop_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
void fb_clear(uint32_t color);
void fb_gradient(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t c1, uint32_t c2);
void fb_wallpaper();
void fb_plot(uint32_t x, uint32_t y, uint32_t color);
void fb_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
void fb_putc(char c, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg);
void fb_print(const char* str, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg);
void fb_reset_cursor_backup();
void fb_hide_cursor();
void fb_show_cursor(int32_t x, int32_t y);
uint32_t fb_get_pixel(uint32_t x, uint32_t y);

