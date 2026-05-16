// Copyright (c) 2023-2026 Christiaan (chris@boreddev.nl)
// This software is released under the GNU General Public License v3.0. See LICENSE file for details.
// This header needs to maintain in any file it is present in, as per the GPL license terms.
#include <stdlib.h>
#include <syscall.h>

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    uint64_t mem[2];
    if (sys_system(SYSTEM_CMD_GET_MEM_INFO, (uint64_t)mem, 0, 0, 0) == 0) {
        printf("Memory Info:\n");
        printf("Total: %d MB\n", (int)(mem[0] / 1024 / 1024));
        printf("Used:  %d MB\n", (int)(mem[1] / 1024 / 1024));
        printf("Free:  %d MB\n", (int)((mem[0] - mem[1]) / 1024 / 1024));
    } else {
        printf("Error: Could not retrieve memory info.\n");
    }
    return 0;
}
