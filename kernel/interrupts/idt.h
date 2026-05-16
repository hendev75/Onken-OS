#pragma once
#include <stdint.h>

struct idt_entry {
    uint16_t isr_low;      // Lower 16 bits of ISR address
    uint16_t kernel_cs;    // GDT Segment Selector
    uint8_t  ist;          // Interrupt Stack Table
    uint8_t  attributes;   // Type and attributes
    uint16_t isr_mid;      // Middle 16 bits of ISR address
    uint32_t isr_high;     // Higher 32 bits of ISR address
    uint32_t reserved;     // Set to 0
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

void idt_init(void);
void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags);
void pic_remap(void);
void pic_send_eoi(uint8_t irq);
