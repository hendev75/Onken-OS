// Copyright (c) 2023-2026 Christiaan (chris@boreddev.nl)
// This software is released under the GNU General Public License v3.0. See LICENSE file for details.
// This header needs to maintain in any file it is present in, as per the GPL license terms.
#include "syscall.h"

int main() {
    const char* msg = "Attempting to crash via null dereference...\n";
    sys_write(1, msg, 45);
    
    volatile int* p = (int*)0;
    *p = 123;
    
    return 0;
}
