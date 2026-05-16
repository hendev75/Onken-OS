#include "app.h"
#include "../string.h"

static app_entry_t* registry[MAX_APPS];
static uint32_t count = 0;

void app_registry_init(void) {
    count = 0;
    for (int i = 0; i < MAX_APPS; i++) {
        registry[i] = 0;
    }
}

void app_register(app_entry_t* app) {
    if (count < MAX_APPS) {
        registry[count++] = app;
    }
}

app_entry_t* app_find(const char* name) {
    for (uint32_t i = 0; i < count; i++) {
        if (strcmp(registry[i]->name, name) == 0) {
            return registry[i];
        }
    }
    return 0;
}

app_entry_t* app_find_by_title(const char* title) {
    for (uint32_t i = 0; i < count; i++) {
        // Compare substring or exact match
        if (strcmp(registry[i]->title, title) == 0) {
            return registry[i];
        }
        // Substring check for titles like "yano - file.txt" matching app "yano"
        int len = strlen(registry[i]->title);
        if (strncmp(registry[i]->title, title, len) == 0) {
            return registry[i];
        }
    }
    return 0;
}

uint32_t app_get_count(void) {
    return count;
}

app_entry_t* app_get_by_index(uint32_t index) {
    if (index < count) {
        return registry[index];
    }
    return 0;
}
