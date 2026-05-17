#include "sound.h"
#include "../kernel/kernel.h"

void sound_play(uint32_t frequency) {
    if (frequency == 0) {
        sound_stop();
        return;
    }
    
    uint32_t div = 1193180 / frequency;
    
    // Set command register to channel 2, access lobyte/hibyte, square wave mode, binary
    outb(0x43, 0xB6);
    
    // Set frequency divisor
    outb(0x42, (uint8_t)(div & 0xFF));
    outb(0x42, (uint8_t)((div >> 8) & 0xFF));
    
    // Enable PC Speaker gate and output
    uint8_t tmp = inb(0x61);
    if (tmp != (tmp | 3)) {
        outb(0x61, tmp | 3);
    }
}

void sound_stop(void) {
    uint8_t tmp = inb(0x61) & 0xFC; // Clear bottom two bits (disable gate and output)
    outb(0x61, tmp);
}

void sound_beep(uint32_t frequency, uint32_t duration_ms) {
    sound_play(frequency);
    
    // Simple busy-wait delay in freestanding kernel
    // (duration_ms * 1000000 approx loops)
    volatile uint32_t delay = duration_ms * 2000000;
    while(delay--);
    
    sound_stop();
}
