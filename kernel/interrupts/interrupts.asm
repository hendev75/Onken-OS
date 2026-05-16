[bits 64]
global idt_load
global irq0_stub
global irq1_stub
global irq12_stub

extern irq0_handler
extern irq1_handler
extern irq12_handler

idt_load:
    lidt [rdi]
    ret

irq0_stub:
    push qword 0          ; error code placeholder
    push qword 32         ; interrupt number
    jmp irq_common

irq1_stub:
    push qword 0
    push qword 33
    jmp irq_common

irq12_stub:
    push qword 0
    push qword 44         ; IRQ12 is 32 + 12 = 44
    jmp irq_common

irq_common:
    ; Save all registers to avoid clobbering C registers
    push rax
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push rbx
    push rbp
    push r12
    push r13
    push r14
    push r15

    mov rdi, rsp          ; Pass register stack frame as pointer argument to C handler
    
    ; Determine interrupt number (offset 15*8 = 120 from top of saved registers)
    mov rsi, [rsp + 120]
    
    cmp rsi, 32
    je .call_irq0
    cmp rsi, 33
    je .call_irq1
    cmp rsi, 44
    je .call_irq12
    jmp .done

.call_irq0:
    call irq0_handler
    jmp .done
.call_irq1:
    call irq1_handler
    jmp .done
.call_irq12:
    call irq12_handler
    jmp .done

.done:
    ; Restore all registers
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    pop rbx
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rax
    
    add rsp, 16          ; clean up interrupt number and error code
    iretq
