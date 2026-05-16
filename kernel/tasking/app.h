#pragma once
#include <stdint.h>

typedef struct {
    const char* name;
    const char* title;
    const char* description;
    void (*init)(void);
    void (*launch)(void);
    void (*draw)(void* self);
    void (*handle_key)(char c);
} app_entry_t;

#define MAX_APPS 32

void app_registry_init(void);
void app_register(app_entry_t* app);
app_entry_t* app_find(const char* name);
app_entry_t* app_find_by_title(const char* title);
uint32_t app_get_count(void);
app_entry_t* app_get_by_index(uint32_t index);
