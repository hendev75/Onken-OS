#include "syscall.h"
#include "../time/pit.h"
#include "../memory/memory.h"
#include "../scheduler/scheduler.h"
#include "../../drivers/ps2.h"
#include "../string.h"

// Forward declaration from shell.c
extern void term_history_add(const char* msg, uint32_t color);
extern void serial_write(const char* str);

void sys_print(const char* msg, uint32_t color) {
    term_history_add(msg, color);
    serial_write(msg);
    serial_write("\n");
}

void sys_sleep(uint32_t ms) {
    sleep(ms);
}

uint64_t sys_uptime(void) {
    return uptime_ms();
}

void sys_get_mem_info(mem_info_t* info) {
    mm_get_info(info);
}

uint32_t sys_get_tasks(task_t* list_out, uint32_t max_tasks) {
    task_t* curr = task_get_list();
    uint32_t count = 0;
    while (curr && count < max_tasks) {
        list_out[count] = *curr; // Copy state
        list_out[count].next = 0; // Break next pointer for safety
        count++;
        curr = curr->next;
    }
    return count;
}

char sys_get_char(void) {
    return ps2_get_last_key();
}

task_t* sys_create_task(const char* name, uint8_t is_kernel) {
    return task_create(name, is_kernel);
}

void sys_destroy_task(uint32_t pid) {
    task_destroy(pid);
}

int sys_is_ctrl_pressed(void) {
    return ps2_is_ctrl_pressed();
}
