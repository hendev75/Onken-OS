#pragma once
#include <stdint.h>

typedef enum {
    TASK_RUNNING,
    TASK_SLEEPING,
    TASK_BLOCKED,
    TASK_ZOMBIE
} task_state_t;

typedef struct task {
    uint32_t pid;
    char name[32];
    task_state_t state;
    uint64_t runtime_ticks;
    uint64_t stack_pointer;
    uint8_t is_kernel;
    struct task* next;
} task_t;

void scheduler_init(void);
task_t* task_create(const char* name, uint8_t is_kernel);
void task_destroy(uint32_t pid);
task_t* task_get_list(void);
uint32_t task_get_count(void);
task_t* task_get_current(void);
void scheduler_tick(void);
