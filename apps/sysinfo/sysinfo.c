#include "sysinfo.h"
#include "../../kernel/tasking/app.h"
#include "../../kernel/syscalls/syscall.h"
#include "../../gui/fb.h"
#include "../../gui/window.h"
#include "../../kernel/string.h"

static app_entry_t sysinfo_app = {
    .name = "sysinfo",
    .title = "System Info",
    .description = "System Specifications",
    .init = sysinfo_init,
    .launch = sysinfo_launch,
    .draw = sysinfo_draw,
    .handle_key = 0
};

void sysinfo_init(void) {
    app_register(&sysinfo_app);
}

void sysinfo_launch(const char* args) {
    (void)args;
    wm_add_window(200, 100, 500, 300, "System Info", sysinfo_draw);
    sys_create_task("sysinfo", 1);
}

void sysinfo_draw(void* self) {
    window_t* w = (window_t*)self;
    
    // ASCII Banner
    fb_print("  ____  _   _ _  _______ _   _ ", w->x + 20, w->y + 40, 0x50E3C2, 0x111111);
    fb_print(" / __ \\| \\ | | |/ / ____| \\ | |", w->x + 20, w->y + 52, 0x50E3C2, 0x111111);
    fb_print("| |  | |  \\| | ' /| |__ |  \\| |", w->x + 20, w->y + 64, 0x50E3C2, 0x111111);
    fb_print("| |  | | . ` |  < |  __|| . ` |", w->x + 20, w->y + 76, 0x50E3C2, 0x111111);
    fb_print("| |__| | |\\  | . \\| |___| |\\  |", w->x + 20, w->y + 88, 0x50E3C2, 0x111111);
    fb_print(" \\____/|_| \\_|_|\\_\\_____|_| \\_|", w->x + 20, w->y + 100, 0x50E3C2, 0x111111);

    fb_print("Made by Ayham & Mason", w->x + 20, w->y + 130, 0xF5A623, 0x111111);
    fb_print("OS Version  : Onken OS Revolution v3.0.0", w->x + 20, w->y + 160, 0xFFFFFF, 0x111111);
    fb_print("Kernel      : Onken Microkernel x86_64", w->x + 20, w->y + 175, 0xFFFFFF, 0x111111);
    fb_print("Graphics    : Desktop Compositor Engine", w->x + 20, w->y + 190, 0xFFFFFF, 0x111111);
    
    char uptime_str[64];
    xsprintf(uptime_str, "Uptime      : %d.%03ds (Ticks: %d)", 
             (uint32_t)(sys_uptime() / 1000), (uint32_t)(sys_uptime() % 1000), (uint32_t)sys_uptime());
    fb_print(uptime_str, w->x + 20, w->y + 205, 0xFFFFFF, 0x111111);
}
