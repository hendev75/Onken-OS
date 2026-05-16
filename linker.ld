OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

SECTIONS
{
    . = 0xffffffff80000000;

    .text : {
        *(.text*)
    }

    .rodata : {
        *(.rodata*)
    }

    .data : {
        *(.data*)
    }

    .requests : {
        KEEP(*(.requests))
    }

    .bss : {
        *(COMMON)
        *(.bss*)
    }
    _kernel_end = .;

    /DISCARD/ : {
        *(.eh_frame)
        *(.note*)
    }
}