#include "fs.h"
#include "string.h"

static vfs_file_t files[MAX_FILES];

vfs_file_t* vfs_get_by_index(int idx) {
    if (idx >= 0 && idx < MAX_FILES) {
        if (files[idx].used) return &files[idx];
    }
    return NULL;
}

int vfs_read(const char* name, char* buf, size_t max_len) {
    vfs_file_t* f = vfs_open(name);
    if (!f) return -1;
    size_t to_copy = f->size;
    if (to_copy > max_len) to_copy = max_len;
    memcpy(buf, f->data, to_copy);
    return (int)to_copy;
}

static void create_demo_bmp(const char* name, int w, int h) {
    // Calculate total size and check it fits
    // BMP rows must be padded to 4-byte boundary
    int row_padding = (4 - (w * 3) % 4) % 4;
    size_t total_pixel_bytes = (size_t)(w * 3 + row_padding) * (size_t)h;
    size_t total_size = 54 + total_pixel_bytes;
    if (total_size > MAX_FILE_SIZE) return; // Safety check!
    
    uint8_t header[54];
    memset(header, 0, 54);
    
    header[0] = 'B';
    header[1] = 'M';
    uint32_t file_size = (uint32_t)total_size;
    memcpy(header + 2, &file_size, 4);
    uint32_t offset = 54;
    memcpy(header + 10, &offset, 4);
    
    uint32_t info_size = 40;
    memcpy(header + 14, &info_size, 4);
    memcpy(header + 18, &w, 4);
    memcpy(header + 22, &h, 4);
    uint16_t planes = 1;
    memcpy(header + 26, &planes, 2);
    uint16_t bpp = 24;
    memcpy(header + 28, &bpp, 2);
    uint32_t img_size = (uint32_t)total_pixel_bytes;
    memcpy(header + 34, &img_size, 4);
    
    vfs_create(name);
    vfs_file_t* f = vfs_open(name);
    if (!f) return;
    
    memcpy(f->data, header, 54);
    size_t data_idx = 54;
    
    // BMP stores rows bottom-to-top
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int cx = w / 2;
            int cy = h / 2;
            int dx = x - cx;
            int dy = y - cy;
            
            // Sunset gradient background
            uint8_t r = (uint8_t)(0x20 + (y * 0x80 / h));
            uint8_t g = (uint8_t)(0x08 + (y * 0x30 / h));
            uint8_t b = (uint8_t)(0x60 - (y * 0x40 / h));
            
            int dist = dx*dx + dy*dy;
            int sun_r = (w / 3);
            
            if (dist < sun_r * sun_r) {
                // Bright golden sun
                r = 0xFF; g = 0xB3; b = 0x00;
                // Retro horizontal scanlines through sun
                if ((y % 6) < 2 && y > cy) {
                    r = 0x80; g = 0x40; b = 0x00;
                }
            } else if (dist < (w/2)*(w/2)) {
                // Orange aura around sun
                r = 0xCC; g = 0x55; b = 0x22;
            }
            
            f->data[data_idx++] = b;
            f->data[data_idx++] = g;
            f->data[data_idx++] = r;
        }
        // Add row padding
        for (int p = 0; p < row_padding; p++) {
            f->data[data_idx++] = 0;
        }
    }
    f->size = data_idx;
}

void vfs_init(void) {
    memset(files, 0, sizeof(files));
    
    // Pre-populate welcome readme
    vfs_create("readme.txt");
    const char* readme_text = 
        "=========================================\n"
        "         WELCOME TO ONKEN OS v3.0        \n"
        "=========================================\n\n"
        "Onken OS is a freestanding modular x86_64\n"
        "operating system written in C and ASM.\n\n"
        "Core Capabilities:\n"
        " - Retro Amiga/AROS Window Compositor\n"
        " - Dynamic Application Registry\n"
        " - PS/2 Keyboard & Mouse Drivers\n"
        " - PIT-driven Preemptive Scheduler\n"
        " - PCI Bus Enumeration\n"
        " - CMOS Real-Time Clock\n"
        " - PC Speaker Sound Driver\n"
        " - BMP Image Viewer\n"
        " - RAM Disk VFS Storage\n\n"
        "Double-click files in Explorer to open!\n"
        "Made by Ayham & Mason.\n";
    vfs_write("readme.txt", readme_text, strlen(readme_text));
              
    // System config
    vfs_create("limine.conf");
    const char* limine_conf = 
        "TIMEOUT=5\n"
        "INTERFACE_RESOLUTION=1280x720x32\n\n"
        ":Onken OS\n"
        "PROTOCOL=limine\n"
        "KERNEL_PATH=boot:///oneko.bin\n";
    vfs_write("limine.conf", limine_conf, strlen(limine_conf));

    // Makefile
    vfs_create("Makefile");
    const char* makefile_content = 
        "CC = gcc\n"
        "CFLAGS = -m64 -ffreestanding -O2 -Wall\n"
        "LDFLAGS = -nostdlib -static\n\n"
        "all: oneko.iso\n"
        "\t@echo 'Build finished!'\n";
    vfs_write("Makefile", makefile_content, strlen(makefile_content));

    // Sample C file
    vfs_create("hello.c");
    const char* hello_c = 
        "#include <stdint.h>\n\n"
        "void main(void) {\n"
        "    // Hello from Onken OS!\n"
        "    volatile char* vga = (char*)0xB8000;\n"
        "    vga[0] = 'H';\n"
        "    vga[1] = 0x0F;\n"
        "}\n";
    vfs_write("hello.c", hello_c, strlen(hello_c));

    // Notes file
    vfs_create("notes.txt");
    const char* notes = 
        "--- Dev Notes ---\n"
        "TODO: Implement FAT32 disk driver\n"
        "TODO: Add AC97 audio support\n"
        "TODO: Ring 3 userland processes\n"
        "DONE: PCI bus enumeration\n"
        "DONE: RTC real-time clock\n"
        "DONE: PC Speaker sound\n";
    vfs_write("notes.txt", notes, strlen(notes));
    
    // Generate a 48x48 sunset BMP wallpaper tile (fits in 16KB)
    create_demo_bmp("sunset.bmp", 48, 48);
}

int vfs_create(const char* name) {
    // Check if already exists first
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used && strcmp(files[i].name, name) == 0) {
            return 0; // Already exists
        }
    }
    for (int i = 0; i < MAX_FILES; i++) {
        if (!files[i].used) {
            strncpy(files[i].name, name, 31);
            files[i].name[31] = '\0';
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
