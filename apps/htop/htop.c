#include "htop.h"
#include "../../kernel/tasking/app.h"
#include "../../kernel/syscalls/syscall.h"
#include "../../gui/fb.h"
#include "../../gui/window.h"
#include "../../kernel/string.h"

// Reference global tick values for real CPU usage calculation
extern volatile uint64_t kernel_ticks;
extern volatile uint64_t idle_ticks;

static app_entry_t htop_app = {
    .name = "htop",
    .title = "htop",
    .description = "Task Manager",
    .init = htop_init,
    .launch = htop_launch,
    .draw = htop_draw,
    .handle_key = 0
};

void htop_init(void) {
    app_register(&htop_app);
}

void htop_launch(void) {
    wm_add_window(150, 150, 600, 320, "htop", htop_draw);
    sys_create_task("htop", 1);
}

void htop_draw(void* self) {
    window_t* w = (window_t*)self;
    fb_print("htop - Task Manager", w->x + 10, w->y + 35, 0x00FF00, 0x111111);

    // 1. Calculate Real CPU Usage
    uint64_t total = kernel_ticks;
    uint64_t idle = idle_ticks;
    uint32_t cpu_usage = 0;
    if (total > 0 && total >= idle) {
        cpu_usage = (uint32_t)((total - idle) * 100 / total);
    }
    if (cpu_usage > 100) cpu_usage = 100;

    // Draw Real CPU Bar
    char cpu_bar[22];
    memset(cpu_bar, ' ', 20);
    cpu_bar[20] = '\0';
    uint32_t cpu_bars_count = cpu_usage / 5; // 20 divisions total
    for (uint32_t i = 0; i < cpu_bars_count; i++) {
        cpu_bar[i] = '|';
    }
    
    char cpu_str[64];
    xsprintf(cpu_str, "CPU  [%s] %d.0%%", cpu_bar, cpu_usage);
    fb_print(cpu_str, w->x + 10, w->y + 55, 0x4A90E2, 0x111111);

    // 2. Fetch Real Memory Info
    mem_info_t mem;
    sys_get_mem_info(&mem);

    // Draw Real Memory Bar
    uint32_t mem_percent = 0;
    if (mem.total_memory > 0) {
        mem_percent = (uint32_t)(mem.used_memory * 100 / mem.total_memory);
    }
    if (mem_percent > 100) mem_percent = 100;

    char mem_bar[22];
    memset(mem_bar, ' ', 20);
    mem_bar[20] = '\0';
    uint32_t mem_bars_count = mem_percent / 5;
    for (uint32_t i = 0; i < mem_bars_count; i++) {
        mem_bar[i] = '|';
    }

    // Convert bytes to MB
    uint32_t used_mb = (uint32_t)(mem.used_memory / 1024 / 1024);
    uint32_t total_mb = (uint32_t)(mem.total_memory / 1024 / 1024);

    char mem_str[64];
    xsprintf(mem_str, "Mem  [%s] %dMB/%dMB (Pages: %d)", mem_bar, used_mb, total_mb, (uint32_t)mem.page_allocations);
    fb_print(mem_str, w->x + 10, w->y + 75, 0xF5A623, 0x111111);

    // Draw Allocations count
    char alloc_str[64];
    xsprintf(alloc_str, "Uptime: %d.%03ds | Active Allocations: %d", 
             (uint32_t)(sys_uptime() / 1000), (uint32_t)(sys_uptime() % 1000), (uint32_t)mem.heap_allocations);
    fb_print(alloc_str, w->x + 10, w->y + 95, 0xBD10E0, 0x111111);

    fb_rect(w->x + 10, w->y + 115, w->w - 20, 2, 0x555555);

    // 3. Draw Real Task List
    fb_print("  PID  COMMAND          STATE     CPU TICKS   TYPE", w->x + 10, w->y + 125, 0xFFFFFF, 0x111111);

    task_t tasks[16];
    uint32_t task_cnt = sys_get_tasks(tasks, 12);
    uint32_t sy = w->y + 145;

    for (uint32_t i = 0; i < task_cnt; i++) {
        char state_char = 'R';
        if (tasks[i].state == TASK_SLEEPING) state_char = 'S';
        else if (tasks[i].state == TASK_BLOCKED) state_char = 'B';
        else if (tasks[i].state == TASK_ZOMBIE) state_char = 'Z';

        char task_line[128];
        xsprintf(task_line, "   %2d  %-16s   %c      %8d     %s", 
                 tasks[i].pid, 
                 tasks[i].name, 
                 state_char, 
                 (uint32_t)tasks[i].runtime_ticks, 
                 tasks[i].is_kernel ? "Kernel" : "User");

        uint32_t text_color = (tasks[i].pid == 42 || strcmp(tasks[i].name, "htop") == 0) ? 0x00FF00 : 0xCCCCCC;
        fb_print(task_line, w->x + 10, sy, text_color, 0x111111);
        sy += 15;
    }
}
