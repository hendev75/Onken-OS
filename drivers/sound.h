#pragma once
#include <stdint.h>

void sound_play(uint32_t frequency);
void sound_stop(void);
void sound_beep(uint32_t frequency, uint32_t duration_ms);
void sound_enqueue(uint32_t frequency, uint32_t duration_ms);
void sound_tick(void);
