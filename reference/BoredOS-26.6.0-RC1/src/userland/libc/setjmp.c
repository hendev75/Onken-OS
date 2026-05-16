#include "setjmp.h"

__attribute__((weak)) int setjmp(jmp_buf env) {
    __asm__ volatile(
        "movq %%rbx, 0(%0)\n\t"
        "movq %%rbp, 8(%0)\n\t"
        "movq %%r12, 16(%0)\n\t"
        "movq %%r13, 24(%0)\n\t"
        "movq %%r14, 32(%0)\n\t"
        "movq %%r15, 40(%0)\n\t"
        "leaq 8(%%rsp), %%rax\n\t"
        "movq %%rax, 48(%0)\n\t"
        "movq (%%rsp), %%rax\n\t"
        "movq %%rax, 56(%0)\n\t"
        :
        : "r"(env)
        : "rax", "memory");
    return 0;
}

__attribute__((weak)) void longjmp(jmp_buf env, int val) {
    int r = (val == 0) ? 1 : val;
    __asm__ volatile(
        "movq 0(%0), %%rbx\n\t"
        "movq 8(%0), %%rbp\n\t"
        "movq 16(%0), %%r12\n\t"
        "movq 24(%0), %%r13\n\t"
        "movq 32(%0), %%r14\n\t"
        "movq 40(%0), %%r15\n\t"
        "movq 48(%0), %%rsp\n\t"
        "movl %1, %%eax\n\t"
        "movq 56(%0), %%rdx\n\t"
        "jmp *%%rdx\n\t"
        :
        : "r"(env), "r"(r)
        : "rax", "rdx", "memory");
    __builtin_unreachable();
}
