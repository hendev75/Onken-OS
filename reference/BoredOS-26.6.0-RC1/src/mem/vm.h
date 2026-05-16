// Copyright (c) 2023-2026 Christiaan (chris@boreddev.nl)
// This software is released under the GNU General Public License v3.0. See LICENSE file for details.
// This header needs to maintain in any file it is present in, as per the GPL license terms.
#ifndef VM_H
#define VM_H

#include <stdint.h>
#include <stdbool.h>

// Simple Stack-Based VM
// Header: "BORDEXE" (7 bytes) + Version (1 byte)

#define VM_MAGIC "BORDEXE"
#define VM_STACK_SIZE 256
#define VM_MEMORY_SIZE (64 * 1024) // 64KB

typedef enum {
    OP_HALT = 0,
    OP_IMM,     // Push immediate (int32)
    OP_LOAD,    // Load from memory (addr) - int32
    OP_STORE,   // Store to memory (addr) - int32
    OP_ADD,     // +
    OP_SUB,     // -
    OP_MUL,     // *
    OP_DIV,     // /
    OP_PRINT,   // Deprecated
    OP_PRITC,   // Deprecated
    OP_JMP,     // Jump (addr)
    OP_JZ,      // Jump if zero
    OP_EQ,      // ==
    OP_NEQ,     // !=
    OP_LT,      // <
    OP_GT,      // >
    OP_LE,      // <=
    OP_GE,      // >=
    OP_SYSCALL, // Call system function (id)
    OP_LOAD8,   // Load byte
    OP_STORE8,  // Store byte
    OP_PUSH_PTR, // Push pointer to data segment (relative to start of mem)
    OP_POP      // Pop and discard top of stack
} OpCode;

// Syscall IDs
typedef enum {
    VM_SYS_EXIT = 0,
    VM_SYS_PRINT_INT,
    VM_SYS_PRINT_CHAR,
    VM_SYS_PRINT_STR,
    VM_SYS_NL,
    VM_SYS_CLS,
    VM_SYS_GETCHAR,
    VM_SYS_STRLEN,
    VM_SYS_STRCMP,
    VM_SYS_STRCPY,
    VM_SYS_STRCAT,
    VM_SYS_MEMSET,
    VM_SYS_MEMCPY,
    VM_SYS_MALLOC,
    VM_SYS_FREE,
    VM_SYS_RAND,
    VM_SYS_SRAND,
    VM_SYS_ABS,
    VM_SYS_MIN,
    VM_SYS_MAX,
    VM_SYS_POW,
    VM_SYS_SQRT,
    VM_SYS_SLEEP,
    VM_SYS_FOPEN,
    VM_SYS_FCLOSE,
    VM_SYS_FREAD,
    VM_SYS_FWRITE,
    VM_SYS_FSEEK,
    VM_SYS_REMOVE,
    VM_SYS_DRAW_PIXEL,
    VM_SYS_DRAW_RECT,
    VM_SYS_DRAW_LINE,
    VM_SYS_DRAW_TEXT,
    VM_SYS_GET_WIDTH,
    VM_SYS_GET_HEIGHT,
    VM_SYS_GET_TIME,
    VM_SYS_KB_HIT,
    VM_SYS_MOUSE_X,
    VM_SYS_MOUSE_Y,
    VM_SYS_MOUSE_STATE,
    VM_SYS_PLAY_SOUND,
    VM_SYS_ATOI,
    VM_SYS_ITOA,
    VM_SYS_PEEK,
    VM_SYS_POKE,
    VM_SYS_EXEC,
    VM_SYS_SYSTEM,
    VM_SYS_STRCHR,
    VM_SYS_MEMCMP,
    VM_SYS_GET_DATE,
    // New Builtins
    VM_SYS_ISALNUM,
    VM_SYS_ISALPHA,
    VM_SYS_ISDIGIT,
    VM_SYS_TOLOWER,
    VM_SYS_TOUPPER,
    VM_SYS_STRNCPY,
    VM_SYS_STRNCAT,
    VM_SYS_STRNCMP,
    VM_SYS_STRSTR,
    VM_SYS_STRRCHR,
    VM_SYS_MEMMOVE
} SyscallID;

int vm_exec(const uint8_t *code, int code_size);

#endif