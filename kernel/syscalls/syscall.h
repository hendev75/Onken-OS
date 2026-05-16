#pragma once
#include <stdint.h>
#include "../memory/memory.h"
#include "../scheduler/scheduler.h"

void sys_print(const char* msg, uint32_t color);
void sys_sleep(uint32_t ms);
uint64_t sys_uptime(void);
void sys_get_mem_info(mem_info_t* info);
uint32_t sys_get_tasks(task_t* list_out, uint32_t max_tasks);
char sys_get_char(void);
task_t* sys_create_task(const char* name, uint8_t is_kernel);
void sys_destroy_task(uint32_t pid);
int sys_is_ctrl_pressed(void);
