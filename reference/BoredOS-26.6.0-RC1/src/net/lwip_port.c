#include <stdint.h>
#include "lwip/opt.h"
#include "lwip/arch.h"

extern volatile uint64_t kernel_ticks;

uint32_t sys_now(void) {

    return (uint32_t)(kernel_ticks * 1000 / 60);
}

sys_prot_t sys_arch_protect(void) {
    uint64_t rflags;
    asm volatile("pushfq; pop %0; cli" : "=r"(rflags));
    return (sys_prot_t)rflags;
}

void sys_arch_unprotect(sys_prot_t pval) {
    asm volatile("push %0; popfq" : : "r"((uint64_t)pval));
}
