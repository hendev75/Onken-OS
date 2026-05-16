#include "kernel.h"
#include "limine.h"
#include "string.h"
#include "../gui/fb.h"
#include "../shell/shell.h"

// Subsystem Headers
#include "interrupts/idt.h"
#include "time/pit.h"
#include "scheduler/scheduler.h"

// Limine Requests
__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

__attribute__((used, section(".requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

__attribute__((used, section(".requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

void ps2_init(void);
void mm_init(void* memmap_req, void* hhdm_req);

// Serial Debug
void serial_putc(char c) {
    while ((inb(0x3F8 + 5) & 0x20) == 0);
    outb(0x3F8, c);
}

void serial_write(const char* str) {
    while (*str) serial_putc(*str++);
}

void log_ok(const char* msg) {
    serial_write("[  OK  ] ");
    serial_write(msg);
    serial_write("\n");
}

void log_info(const char* msg) {
    serial_write("[ INFO ] ");
    serial_write(msg);
    serial_write("\n");
}

void kernel_main(void) {
    // Initial serial init
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x80);
    outb(0x3F8 + 0, 0x03);
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x03);
    outb(0x3F8 + 2, 0xC7);
    outb(0x3F8 + 4, 0x0B);

    log_info("Onken OS booting...");
    log_info("Limine protocol detected");

    // Check framebuffer
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        serial_write("No framebuffer available!\n");
        for(;;) asm("hlt");
    }

    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
    
    fb_init_raw(fb->address, fb->width, fb->height, fb->pitch);
    log_ok("Framebuffer initialized");
    log_ok("GDT initialized");
    log_ok("Paging ready");

    mm_init((void*)&memmap_request, (void*)&hhdm_request);
    log_ok("Memory manager initialized");

    fb_alloc_buffers();
    fb_clear(0x111111);
    fb_render_wallpaper();
    log_ok("GUI Buffers initialized");

    // Initialize Interrupts, PIT, and Scheduler
    idt_init();
    log_ok("Interrupt Descriptor Table (IDT) active");

    pit_init(1000); // 1000Hz (1ms ticks)
    log_ok("PIT hardware timer active");

    scheduler_init();
    log_ok("Task Scheduler active");

    ps2_init();
    log_ok("PS/2 Input active (Interrupt-driven)");
    
    log_ok("Welcome to Onken OS!");

    shell_loop();
}