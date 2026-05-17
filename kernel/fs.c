#include "fs.h"
#include "string.h"

static vfs_file_t files[MAX_FILES];

vfs_file_t* vfs_get_by_index(int idx) {
    if (idx >= 0 && idx < MAX_FILES) {
        if (files[idx].used) return &files[idx];
    }
    return NULL;
}

static void create_demo_bmp(const char* name, int w, int h) {
    // BMP header array (54 bytes)
    uint8_t header[54];
    memset(header, 0, 54);
    
    // File signature & size
    header[0] = 'B';
    header[1] = 'M';
    uint32_t file_size = 54 + w * h * 3;
    memcpy(header + 2, &file_size, 4);
    uint32_t offset = 54;
    memcpy(header + 10, &offset, 4);
    
    // Info header (40 bytes)
    uint32_t info_size = 40;
    memcpy(header + 14, &info_size, 4);
    memcpy(header + 18, &w, 4);
    memcpy(header + 22, &h, 4);
    uint16_t planes = 1;
    memcpy(header + 26, &planes, 2);
    uint16_t bpp = 24;
    memcpy(header + 28, &bpp, 2);
    
    vfs_create(name);
    vfs_file_t* f = vfs_open(name);
    if (!f) return;
    
    memcpy(f->data, header, 54);
    size_t data_idx = 54;
    
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            // Compute elegant concentric retro-sunset target ring pattern
            int cx = w / 2;
            int cy = h / 2;
            int dx = x - cx;
            int dy = y - cy;
            
            // Sunset color gradient background
            uint8_t r = 0x80 - (y * 0x60 / h); // Dark red to orange gradient
            uint8_t g = 0x20 + (y * 0x30 / h);
            uint8_t b = 0x10;
            
            int dist = dx*dx + dy*dy;
            
            // Render retro-futuristic horizon stripes
            if (dist < (w/3)*(w/3)) {
                // Gold glowing sun
                r = 0xD4; g = 0x90; b = 0x10;
                // Add retro horizon line cut-offs!
                if ((y % 12) < 3 && y > cy) {
                    r = 0x20; g = 0x10; b = 0x08; // dark sunset stripes
                }
            } else if (dist < (w/2)*(w/2) && dist >= (w/3)*(w/3) + 20) {
                // Indigo/purple background sunset aura
                r = 0x4B; g = 0x00; b = 0x82;
            }
            
            f->data[data_idx++] = b; // Blue
            f->data[data_idx++] = g; // Green
            f->data[data_idx++] = r; // Red
        }
    }
    f->size = data_idx;
}

void vfs_init(void) {
    memset(files, 0, sizeof(files));
    
    // Pre-populate welcome readme text
    vfs_create("readme.txt");
    const char* readme_text = 
        "=========================================\n"
        "         WELCOME TO ONKEN OS v3.0         \n"
        "=========================================\n\n"
        "Onken OS is a freestanding modular x86_64\n"
        "operating system designed primarily in C.\n\n"
        "Core Capabilities:\n"
        " - Dynamic Window Compositor (Amiga style)\n"
        " - Dynamic modular applications manager\n"
        " - PS/2 input keyboard & mouse drivers\n"
        " - Preemptive scheduler (PIT-driven task loops)\n"
        " - PC Speaker retro chiptune player app\n"
        " - Built-in procedural BMP Image Viewer app\n"
        " - Dynamic RAM Disk VFS storage system\n\n"
        "Double click files in File Explorer to launch!\n"
        "Made by Ayham & Mason. Enjoy retro power!";
    vfs_write("readme.txt", readme_text, strlen(readme_text));
              
    // Pre-populate system config file
    vfs_create("limine.conf");
    const char* limine_conf = 
        "TIMEOUT=5\n"
        "INTERFACE_RESOLUTION=1280x720x32\n\n"
        ":Onken OS\n"
        "PROTOCOL=limine\n"
        "KERNEL_PATH=boot:///oneko.bin\n";
    vfs_write("limine.conf", limine_conf, strlen(limine_conf));

    // Pre-populate make configuration
    vfs_create("Makefile");
    const char* makefile_content = 
        "CC = gcc\n"
        "CFLAGS = -m64 -ffreestanding -O2 -Wall\n"
        "LDFLAGS = -nostdlib -static\n\n"
        "all: oneko.iso\n"
        "\t@echo 'Build finished!'\n";
    vfs_write("Makefile", makefile_content, strlen(makefile_content));
    
    // Generate beautiful sunset wallpaper image in VFS RAM disk!
    create_demo_bmp("sunset.bmp", 64, 64);
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
