#pragma once
#include <stdint.h>

typedef struct {
    uint64_t total_memory;
    uint64_t used_memory;
    uint64_t free_memory;
    uint64_t heap_allocations;
    uint64_t page_allocations;
    uint64_t heap_used_bytes;
    uint64_t heap_start;
    uint64_t heap_end;
} mem_info_t;

void mm_get_info(mem_info_t* info);
void* palloc(void); // page allocator simulator/allocator
