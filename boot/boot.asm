bits 64
global _start
extern kernel_main

section .text
_start:
    cli
    ; Limine provides a stack, but we can set our own if we want.
    ; For now, use Limine's stack.
    call kernel_main
    hlt
.loop:
    jmp .loop
