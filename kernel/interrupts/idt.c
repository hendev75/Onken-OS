#include "idt.h"
#include "../kernel.h"

extern void idt_load(struct idt_ptr* ptr);
extern void irq0_stub(void);
extern void irq1_stub(void);
extern void irq12_stub(void);

static struct idt_entry idt[256];
static struct idt_ptr idtp;

void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags) {
    idt[num].isr_low = (uint16_t)(base & 0xFFFF);
    idt[num].kernel_cs = sel;
    idt[num].ist = 0;
    idt[num].attributes = flags;
    idt[num].isr_mid = (uint16_t)((base >> 16) & 0xFFFF);
    idt[num].isr_high = (uint32_t)((base >> 32) & 0xFFFFFFFF);
    idt[num].reserved = 0;
}

void pic_remap(void) {
    // ICW1 - Initialize Master and Slave
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    // ICW2 - Remap Master to 0x20 (32), Slave to 0x28 (40)
    outb(0x21, 0x20);
    outb(0xA1, 0x28);

    // ICW3 - Tell Master that Slave is at IRQ2, and Slave its identity
    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    // ICW4 - Environment info
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // Mask/Unmask: Unmask PIT (IRQ0), Keyboard (IRQ1), Cascade (IRQ2) on Master
    // And Mouse (IRQ12) on Slave.
    // Master: Unmask IRQ0, IRQ1, IRQ2 (cascade) -> 0b11111000 = 0xF8
    outb(0x21, 0xF8);
    // Slave: Unmask IRQ12 -> 0b11101111 = 0xEF
    outb(0xA1, 0xEF);
}

void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(0xA0, 0x20);
    }
    outb(0x20, 0x20);
}

void idt_init(void) {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint64_t)&idt;

    // Clear IDT
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    uint16_t cs;
    asm volatile("mov %%cs, %0" : "=r"(cs));

    pic_remap();

    // Register our interrupts (0x8E = Present, ring 0, 64-bit Interrupt Gate)
    idt_set_gate(32, (uint64_t)irq0_stub, cs, 0x8E); // PIT timer
    idt_set_gate(33, (uint64_t)irq1_stub, cs, 0x8E); // Keyboard
    idt_set_gate(44, (uint64_t)irq12_stub, cs, 0x8E); // Mouse (IRQ12 = 32 + 12 = 44)

    idt_load(&idtp);

    // Enable interrupts
    asm volatile("sti");
}
