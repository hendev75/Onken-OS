// Copyright (c) 2023-2026 Christiaan (chris@boreddev.nl)
// This software is released under the GNU General Public License v3.0. See LICENSE file for details.
#include "../libc/syscall.h"
#include "../libc/stdio.h"
#include "../libc/string.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: rescan /dev/DEVICE\n");
        return 1;
    }

    const char *devname = argv[1];
    if (devname[0] == '/' && devname[1] == 'd' && devname[2] == 'e' && devname[3] == 'v' && devname[4] == '/')
        devname += 5;

    printf("Rescanning /dev/%s...\n", devname);
    int ret = sys_disk_rescan(devname);
    if (ret != 0) {
        printf("[ERROR] Rescan failed (ret=%d)\n", ret);
        return 1;
    }

    printf("Done.\n");
    return 0;
}
