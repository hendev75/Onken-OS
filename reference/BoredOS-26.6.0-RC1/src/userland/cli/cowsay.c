// Copyright (c) 2023-2026 Christiaan (chris@boreddev.nl)
// This software is released under the GNU General Public License v3.0. See LICENSE file for details.
// This header needs to maintain in any file it is present in, as per the GPL license terms.
#include <stdlib.h>
#include <syscall.h>

int main(int argc, char **argv) {
    char *msg = (char*)"Bored!";
    if (argc > 1) {
        msg = argv[1];
    }
    
    size_t len = strlen(msg);
    
    printf(" ");
    for(size_t i=0; i<len+2; i++) printf("_");
    printf("\n< %s >\n ", msg);
    for(size_t i=0; i<len+2; i++) printf("-");
    printf("\n");
    printf("        \\   ^__^\n");
    printf("         \\  (oo)\\_______\n");
    printf("            (__)\\       )\\/\\\n");
    printf("                ||----w |\n");
    printf("                ||     ||\n\n");
    
    return 0;
}
