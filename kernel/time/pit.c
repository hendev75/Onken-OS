#include "pit.h"
#include "../kernel.h"
#include "../interrupts/idt.h"
#include "../scheduler/scheduler.h"
#include "../../drivers/sound.h"

volatile uint64_t kernel_ticks = 0;
volatile uint64_t idle_ticks = 0;
uint32_t timer_frequency = 1000;

void pit_init(uint32_t frequency) {
    timer_frequency = frequency;
    uint32_t divisor = 1193182 / frequency;
    
    // Command Register 0x43:
    // Bits 7-6: Select Channel 0
    // Bits 5-4: Access Mode (lobyte/hibyte)
    // Bits 3-1: Mode 3 (Square Wave Generator)
    // Bit 0: Binary Mode
    outb(0x43, 0x36);
    
    // Data Register 0x40 (Channel 0 data):
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}

uint64_t uptime_ms(void) {
    return (kernel_ticks * 1000) / timer_frequency;
}

void sleep(uint32_t ms) {
    uint64_t target_ticks = kernel_ticks + (uint64_t)ms * timer_frequency / 1000;
    while (kernel_ticks < target_ticks) {
        // Wait, wait, let the CPU rest a bit
        asm volatile("hlt");
    }
}

// C handler for Timer Interrupt (IRQ 0)
void irq0_handler(void* stack_frame) {
    (void)stack_frame;
    kernel_ticks++;
    
    // Process scheduler accounting
    scheduler_tick();
    
    // Process sound queue
    sound_tick();
    
    // Send EOI to PIC
    pic_send_eoi(0);
}
