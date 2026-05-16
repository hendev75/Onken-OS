#pragma once
#include <stdint.h>

extern volatile uint64_t kernel_ticks;
extern volatile uint64_t idle_ticks;
extern uint32_t timer_frequency;

void pit_init(uint32_t frequency);
uint64_t uptime_ms(void);
void sleep(uint32_t ms);
