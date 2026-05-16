#include "scheduler.h"
#include "../kernel.h"
#include "../string.h"

static task_t* task_list = 0;
static task_t* current_task = 0;
static uint32_t next_pid = 1;

void scheduler_init(void) {
    task_list = 0;
    
    // Create base system tasks
    task_t* init = task_create("init", 1);
    task_create("window_manager", 1);
    task_create("terminal", 1);
    
    current_task = init;
}

task_t* task_create(const char* name, uint8_t is_kernel) {
    task_t* t = (task_t*)kmalloc(sizeof(task_t));
    t->pid = next_pid++;
    strncpy(t->name, name, 31);
    t->state = TASK_RUNNING;
    t->runtime_ticks = 0;
    t->stack_pointer = 0;
    t->is_kernel = is_kernel;
    t->next = 0;
    
    // Add to linked list
    if (!task_list) {
        task_list = t;
    } else {
        task_t* curr = task_list;
        while (curr->next) {
            curr = curr->next;
        }
        curr->next = t;
    }
    
    return t;
}

void task_destroy(uint32_t pid) {
    if (!task_list) return;
    
    if (task_list->pid == pid) {
        task_t* to_free = task_list;
        task_list = task_list->next;
        to_free->state = TASK_ZOMBIE;
        // In a real OS we'd free memory, but kmalloc is a simple bump allocator
        return;
    }
    
    task_t* curr = task_list;
    while (curr->next) {
        if (curr->next->pid == pid) {
            task_t* to_free = curr->next;
            curr->next = curr->next->next;
            to_free->state = TASK_ZOMBIE;
            return;
        }
        curr = curr->next;
    }
}

task_t* task_get_list(void) {
    return task_list;
}

uint32_t task_get_count(void) {
    uint32_t count = 0;
    task_t* curr = task_list;
    while (curr) {
        if (curr->state != TASK_ZOMBIE) {
            count++;
        }
        curr = curr->next;
    }
    return count;
}

task_t* task_get_current(void) {
    return current_task;
}

void scheduler_tick(void) {
    // Increment ticks of the currently running task
    if (current_task) {
        current_task->runtime_ticks++;
    }
    
    // Increment idle ticks if the current task is idle (e.g. init)
    extern volatile uint64_t idle_ticks;
    if (current_task && strcmp(current_task->name, "init") == 0) {
        idle_ticks++;
    }
}
