// Copyright (c) 2023-2026 Christiaan (chris@boreddev.nl)
// This software is released under the GNU General Public License v3.0. See LICENSE file for details.
// This header needs to maintain in any file it is present in, as per the GPL license terms.
#include <stdlib.h>
#include <syscall.h>

int main(int argc, char **argv) {
    if (argc < 4) {
        printf("Usage: math <num1> <op> <num2>\n");
        printf("Ops: + - * /\n");
        return 0;
    }
    
    int n1 = atoi(argv[1]);
    int n2 = atoi(argv[3]);
    char op = argv[2][0];
    int res = 0;
    
    if (op == '+') res = n1 + n2;
    else if (op == '-') res = n1 - n2;
    else if (op == '*') res = n1 * n2;
    else if (op == '/') {
        if (n2 == 0) { printf("Error: Div by zero\n"); return 1; }
        res = n1 / n2;
    }
    else { printf("Error: Unknown op %c\n", op); return 1; }
    
    printf("%d %c %d = %d\n", n1, op, n2, res);
    return 0;
}
