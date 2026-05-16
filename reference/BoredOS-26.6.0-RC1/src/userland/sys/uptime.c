// Copyright (c) 2023-2026 Christiaan (chris@boreddev.nl)
// This software is released under the GNU General Public License v3.0. See LICENSE file for details.
// This header needs to maintain in any file it is present in, as per the GPL license terms.
#include <stdlib.h>
#include <syscall.h>

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    int fd = sys_open("/proc/uptime", "r");
    if (fd < 0) return 1;
    char buf[128];
    int bytes = sys_read(fd, buf, 127);
    sys_close(fd);
    if (bytes <= 0) return 1;
    buf[bytes] = 0;

    int seconds = atoi(buf);
    int minutes = seconds / 60;
    int hours = minutes / 60;
    int days = hours / 24;
    
    printf("Uptime: %d days, %d hours, %d minutes, %d seconds\n", 
           (int)days, (int)(hours % 24), (int)(minutes % 60), (int)(seconds % 60));
    
    return 0;
}
