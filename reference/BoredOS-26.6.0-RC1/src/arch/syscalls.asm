; Copyright (c) 2023-2026 Christiaan (chris@boreddev.nl)
; This software is released under the GNU General Public License v3.0. See LICENSE file for details.
; This header needs to maintain in any file it is present in, as per the GPL license terms.
global syscall_entry
extern syscall_handler_c

section .text

; Syscall ABI:
; RAX = syscall_num
; RDI = arg1
; RSI = arg2
; RDX = arg3
; R10 = arg4
; R8  = arg5
; R9  = arg6

syscall_entry:
    swapgs 
    
    mov [gs:40], rsp
    mov rsp, [gs:48]

    ; 2. Build iretq frame 
    push 0x1B           ; SS (User Data)
    push qword [gs:40]  ; RSP
    push r11            ; RFLAGS (captured by syscall)
    push 0x23           ; CS (User Code)
    push rcx            ; RIP (return address from syscall)
    
    push 0              ; err_code
    push 0              ; int_no (can be used for syscall vector)
    
    ; 3. Save all registers in registers_t order
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    
    ; Save SSE/FPU state
    sub rsp, 512
    fxsave [rsp]

    ; 4. Call C handler with registers_t*
    mov rdi, rsp
    call syscall_handler_c

    ; 5. Switch to the resulting RSP (might be different if task switched)
    mov rsp, rax

    ; Restore SSE/FPU state
    fxrstor [rsp]
    add rsp, 512

    ; 6. Restore and return via iretq
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    add rsp, 16 ; drop int_no/err_code
    
    swapgs 
    iretq

section .bss
