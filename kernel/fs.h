#pragma once
#include <stdint.h>
#include <stddef.h>

#define MAX_FILES 64
#define MAX_FILE_SIZE 4096

typedef struct {
    char name[32];
    char data[MAX_FILE_SIZE];
    size_t size;
    uint8_t used;
} vfs_file_t;

void vfs_init(void);
int vfs_create(const char* name);
vfs_file_t* vfs_open(const char* name);
int vfs_write(const char* name, const char* data, size_t len);
void vfs_list(void (*print_fn)(const char*));
int vfs_remove(const char* name);
