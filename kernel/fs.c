#include "fs.h"
#include "string.h"

static vfs_file_t files[MAX_FILES];

void vfs_init(void) {
    memset(files, 0, sizeof(files));
}

int vfs_create(const char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (!files[i].used) {
            strncpy(files[i].name, name, 31);
            files[i].used = 1;
            files[i].size = 0;
            return 0;
        }
    }
    return -1;
}

vfs_file_t* vfs_open(const char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used && strcmp(files[i].name, name) == 0) {
            return &files[i];
        }
    }
    return NULL;
}

int vfs_write(const char* name, const char* data, size_t len) {
    vfs_file_t* f = vfs_open(name);
    if (!f) return -1;
    if (len > MAX_FILE_SIZE) len = MAX_FILE_SIZE;
    memcpy(f->data, data, len);
    f->size = len;
    return 0;
}

void vfs_list(void (*print_fn)(const char*)) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used) {
            print_fn(files[i].name);
        }
    }
}

int vfs_remove(const char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used && strcmp(files[i].name, name) == 0) {
            files[i].used = 0;
            return 0;
        }
    }
    return -1;
}
