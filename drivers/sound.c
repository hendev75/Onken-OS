#include "sound.h"
#include "../kernel/kernel.h"
#include "../kernel/time/pit.h"

typedef struct {
    uint32_t freq;
    uint32_t duration_ms;
} note_t;

#define SOUND_QUEUE_SIZE 16
static note_t sound_queue[SOUND_QUEUE_SIZE];
static int sound_q_head = 0;
static int sound_q_tail = 0;
static uint64_t current_note_end_ms = 0;
static int sound_is_playing = 0;

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
    sound_is_playing = 0;
}

void sound_beep(uint32_t frequency, uint32_t duration_ms) {
    sound_play(frequency);
    sleep(duration_ms);
    sound_stop();
}

void sound_enqueue(uint32_t frequency, uint32_t duration_ms) {
    int next_tail = (sound_q_tail + 1) % SOUND_QUEUE_SIZE;
    if (next_tail != sound_q_head) {
        sound_queue[sound_q_tail].freq = frequency;
        sound_queue[sound_q_tail].duration_ms = duration_ms;
        sound_q_tail = next_tail;
    }
}

void sound_tick(void) {
    if (sound_is_playing) {
        if (uptime_ms() >= current_note_end_ms) {
            sound_stop();
        }
    } else {
        if (sound_q_head != sound_q_tail) {
            note_t note = sound_queue[sound_q_head];
            sound_q_head = (sound_q_head + 1) % SOUND_QUEUE_SIZE;
            
            if (note.freq > 0) {
                sound_play(note.freq);
                sound_is_playing = 1;
            } else {
                sound_stop(); // Silence rest
                sound_is_playing = 1; // Fake playing for silence duration
            }
            current_note_end_ms = uptime_ms() + note.duration_ms;
        }
    }
}
