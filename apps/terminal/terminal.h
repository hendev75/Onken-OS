#pragma once

void terminal_init(void);
void terminal_launch(const char* args);
void terminal_draw(void* self);
void terminal_handle_key(char c);
