#pragma once
#include <stdint.h>

extern int32_t mouse_x;
extern int32_t mouse_y;
extern uint8_t mouse_b_left;
extern uint8_t mouse_b_right;

void ps2_init(void);
void ps2_poll(void);
char ps2_get_last_key(void);
int ps2_is_alt_pressed(void);
