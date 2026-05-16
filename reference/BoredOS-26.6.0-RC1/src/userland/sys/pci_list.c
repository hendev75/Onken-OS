// Copyright (c) 2023-2026 Christiaan (chris@boreddev.nl)
// This software is released under the GNU General Public License v3.0. See LICENSE file for details.
// This header needs to maintain in any file it is present in, as per the GPL license terms.
#include <stdlib.h>
#include <syscall.h>

typedef struct {
    uint16_t vendor;
    uint16_t device;
    uint8_t class_code;
    uint8_t subclass;
} pci_info_t;

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    int count = sys_system(SYSTEM_CMD_PCI_LIST, 0, 0, 0, 0);
    if (count < 0) {
        printf("Error: Could not retrieve PCI device count.\n");
        return 1;
    }
    
    printf("PCI Devices (%d found):\n", count);
    printf("---------------------------\n");
    for (int i = 0; i < count; i++) {
        pci_info_t info;
        if (sys_system(SYSTEM_CMD_PCI_LIST, (uint64_t)&info, i, 0, 0) == 0) {
            printf("[%d] Vendor:%04x Device:%04x Class:%02x Sub:%02x\n", 
                   i, info.vendor, info.device, info.class_code, info.subclass);
        }
    }
    
    return 0;
}
