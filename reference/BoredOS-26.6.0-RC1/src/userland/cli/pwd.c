// Copyright (c) 2023-2026 Christiaan (chris@boreddev.nl)
// This software is released under the GNU General Public License v3.0. See LICENSE file for details.
// This header needs to maintain in any file it is present in, as per the GPL license terms.
#include <stdlib.h>
#include <syscall.h>

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    char path[256];
    if (getcwd(path, sizeof(path))) {
        printf("%s\n", path);
    } else {
        printf("Error: Could not get current directory\n");
        return 1;
    }
    return 0;
}
