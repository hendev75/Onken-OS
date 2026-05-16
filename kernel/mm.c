#include "kernel.h"

#include "limine.h"

extern char _kernel_end[];
static uint8_t* heap_ptr = 0;

void mm_init(void* memmap_req_ptr, void* hhdm_req_ptr) {
    struct limine_memmap_request* memmap_req = (struct limine_memmap_request*)memmap_req_ptr;
    struct limine_hhdm_request* hhdm_req = (struct limine_hhdm_request*)hhdm_req_ptr;

    uint64_t hhdm_offset = hhdm_req->response->offset;
    struct limine_memmap_response* resp = memmap_req->response;

    // Find the largest usable memory segment
    uint64_t max_len = 0;
    uint64_t best_base = 0;

    for (uint64_t i = 0; i < resp->entry_count; i++) {
        struct limine_memmap_entry* entry = resp->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            if (entry->length > max_len) {
                max_len = entry->length;
                best_base = entry->base;
            }
        }
    }

    if (max_len > 0) {
        // Point heap to the HHDM address of the largest usable region
        heap_ptr = (uint8_t*)(best_base + hhdm_offset);
    } else {
        // Fallback
        heap_ptr = (uint8_t*)_kernel_end;
    }
}

void* kmalloc(uint32_t size) {
    void* ptr = heap_ptr;
    heap_ptr += (size + 15) & ~15; // 16-byte align
    return ptr;
}