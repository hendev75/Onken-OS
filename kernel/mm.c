#include "kernel.h"
#include "limine.h"
#include "memory/memory.h"

extern char _kernel_end[];
static uint8_t* heap_ptr = 0;

static uint64_t total_mem_bytes = 0;
static uint64_t kmalloc_count = 0;
static uint64_t kmalloc_bytes = 0;
static uint64_t page_alloc_count = 0;
static uint64_t heap_start_addr = 0;

void mm_init(void* memmap_req_ptr, void* hhdm_req_ptr) {
    struct limine_memmap_request* memmap_req = (struct limine_memmap_request*)memmap_req_ptr;
    struct limine_hhdm_request* hhdm_req = (struct limine_hhdm_request*)hhdm_req_ptr;

    uint64_t hhdm_offset = hhdm_req->response->offset;
    struct limine_memmap_response* resp = memmap_req->response;

    // Find the largest usable memory segment and sum total usable memory
    uint64_t max_len = 0;
    uint64_t best_base = 0;
    total_mem_bytes = 0;

    for (uint64_t i = 0; i < resp->entry_count; i++) {
        struct limine_memmap_entry* entry = resp->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            total_mem_bytes += entry->length;
            if (entry->length > max_len) {
                max_len = entry->length;
                best_base = entry->base;
            }
        }
    }

    if (max_len > 0) {
        heap_ptr = (uint8_t*)(best_base + hhdm_offset);
    } else {
        heap_ptr = (uint8_t*)_kernel_end;
    }
    
    heap_start_addr = (uint64_t)heap_ptr;
}

void* kmalloc(uint32_t size) {
    void* ptr = heap_ptr;
    uint32_t aligned_size = (size + 15) & ~15; // 16-byte align
    heap_ptr += aligned_size;
    
    kmalloc_count++;
    kmalloc_bytes += aligned_size;
    
    return ptr;
}

void* palloc(void) {
    // Page allocator (4096-byte aligned page)
    uint64_t current = (uint64_t)heap_ptr;
    uint64_t aligned = (current + 4095) & ~4095;
    heap_ptr = (uint8_t*)(aligned + 4096);
    
    page_alloc_count++;
    kmalloc_bytes += (heap_ptr - (uint8_t*)current);
    
    return (void*)aligned;
}

void mm_get_info(mem_info_t* info) {
    info->total_memory = total_mem_bytes;
    info->heap_used_bytes = kmalloc_bytes;
    info->heap_allocations = kmalloc_count;
    info->page_allocations = page_alloc_count;
    info->used_memory = kmalloc_bytes;
    info->free_memory = total_mem_bytes - info->used_memory;
    info->heap_start = heap_start_addr;
    info->heap_end = (uint64_t)heap_ptr;
}