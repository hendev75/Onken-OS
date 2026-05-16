// Copyright (c) 2026 maro (whitehai11)
// This software is released under the GNU General Public License v3.0. See LICENSE file for details.
// This header needs to maintain in any file it is present in, as per the GPL license terms.
// BOREDOS_APP_DESC: Search for text inside a file.

#include "../libc/syscall.h"
#include "../libc/stdlib.h"
#include "../libc/string.h"
#include "../libc/stdio.h"

#define READ_BUF_SIZE 4096
#define LINE_BUF_SIZE 1024
#define MAX_PATH      512
#define MAX_ENTRIES   256

// Flags
static int g_show_numbers  = 0;
static int g_ignore_case   = 0;
static int g_count_only    = 0;
static int g_invert        = 0;  // -v
static int g_files_only    = 0;  // -l
static int g_word_match    = 0;  // -w
static int g_line_match    = 0;  // -x
static int g_recursive     = 0;  // -r / -R
static int g_multi_file    = 0;  // more than one file → prefix output with filename

static const char *g_pattern = NULL;

// Total match count across all files (used for -c with -r)
static int g_total_matches = 0;

static int sc_strcmp(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

static void print_usage(void) {
    printf("Usage: grep [options] <pattern> <file> [file...]\n");
    printf("\n");
    printf("Search for text inside files.\n");
    printf("\n");
    printf("Options:\n");
    printf("  -n      Show line numbers\n");
    printf("  -i      Case-insensitive search\n");
    printf("  -c      Print match count only\n");
    printf("  -v      Invert match (print non-matching lines)\n");
    printf("  -l      Print only filenames with matches\n");
    printf("  -w      Match whole words only\n");
    printf("  -x      Match whole lines only\n");
    printf("  -r, -R  Recursive search in directories\n");
    printf("  -h      Show this help\n");
}

static char to_lower(char c) {
    if (c >= 'A' && c <= 'Z') return c + 32;
    return c;
}

static int is_word_char(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '_';
}

/* Check if needle appears in haystack at position i as a whole word */
static int match_at(const char *haystack, int i, int h_len,
                    const char *needle, int n_len, int ignore_case) {
    for (int j = 0; j < n_len; j++) {
        char h = ignore_case ? to_lower(haystack[i + j]) : haystack[i + j];
        char n = ignore_case ? to_lower(needle[j])       : needle[j];
        if (h != n) return 0;
    }
    if (g_word_match) {
        if (i > 0 && is_word_char(haystack[i - 1])) return 0;
        if (i + n_len < h_len && is_word_char(haystack[i + n_len])) return 0;
    }
    return 1;
}

static int str_contains(const char *haystack, const char *needle, int ignore_case) {
    int h_len = (int)strlen(haystack);
    int n_len = (int)strlen(needle);

    if (n_len == 0) return 1;
    if (n_len > h_len) return 0;

    for (int i = 0; i <= h_len - n_len; i++) {
        if (match_at(haystack, i, h_len, needle, n_len, ignore_case))
            return 1;
    }
    return 0;
}

static int line_matches(const char *line) {
    if (g_line_match) {
        /* Whole line must equal pattern */
        int h_len = (int)strlen(line);
        int n_len = (int)strlen(g_pattern);
        if (h_len != n_len) return 0;
        return match_at(line, 0, h_len, g_pattern, n_len, g_ignore_case);
    }
    return str_contains(line, g_pattern, g_ignore_case);
}

/* Grep a single open file descriptor, printing results prefixed by filename if needed */
static int grep_fd(int fd, const char *filename) {
    static char read_buf[READ_BUF_SIZE];
    static char line[LINE_BUF_SIZE];
    int line_pos  = 0;
    int line_num  = 0;
    int match_cnt = 0;

    /* Helper: process one complete line */
    #define PROCESS_LINE() do { \
        line[line_pos] = '\0'; \
        line_num++; \
        int matched = line_matches(line); \
        if (g_invert) matched = !matched; \
        if (matched) { \
            match_cnt++; \
            if (!g_count_only && !g_files_only) { \
                if (g_multi_file) printf("%s:", filename); \
                if (g_show_numbers) printf("%d:", line_num); \
                printf("%s\n", line); \
            } \
        } \
        line_pos = 0; \
    } while (0)

    while (1) {
        int bytes = sys_read(fd, read_buf, READ_BUF_SIZE);
        if (bytes <= 0) break;

        for (int i = 0; i < bytes; i++) {
            char c = read_buf[i];
            if (c == '\n' || line_pos >= LINE_BUF_SIZE - 1) {
                PROCESS_LINE();
            } else if (c != '\r') {
                line[line_pos++] = c;
            }
        }
    }
    if (line_pos > 0) PROCESS_LINE();

    #undef PROCESS_LINE

    if (g_count_only)
        printf("%s%d\n", g_multi_file ? filename : "", match_cnt > 0 ? match_cnt : 0);

    if (g_files_only && match_cnt > 0)
        printf("%s\n", filename);

    g_total_matches += match_cnt;
    return match_cnt;
}

static int grep_file(const char *path) {
    int fd = sys_open(path, "r");
    if (fd < 0) {
        printf("grep: cannot open '%s'\n", path);
        return 0;
    }
    int n = grep_fd(fd, path);
    sys_close(fd);
    return n;
}

static void grep_recursive(const char *path) {
    FAT32_FileInfo info;
    if (sys_get_file_info(path, &info) < 0) {
        printf("grep: cannot access '%s'\n", path);
        return;
    }

    if (!info.is_directory) {
        grep_file(path);
        return;
    }

    FAT32_FileInfo entries[MAX_ENTRIES];
    int count = sys_list(path, entries, MAX_ENTRIES);
    if (count < 0) return;

    for (int i = 0; i < count; i++) {
        const char *name = entries[i].name;
        if (name[0] == '.' && (name[1] == '\0' || (name[1] == '.' && name[2] == '\0')))
            continue;

        char full[MAX_PATH];
        int plen = (int)strlen(path);
        int nlen = (int)strlen(name);
        if (plen + 1 + nlen + 1 > MAX_PATH) continue;

        int slash = (plen == 1 && path[0] == '/') ? 0 : 1;
        for (int j = 0; j < plen; j++) full[j] = path[j];
        if (slash) full[plen] = '/';
        for (int j = 0; j <= nlen; j++) full[plen + slash + j] = name[j];

        if (entries[i].is_directory)
            grep_recursive(full);
        else
            grep_file(full);
    }
}

int main(int argc, char **argv) {
    int arg_offset = 1;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') break;
        if (sc_strcmp(argv[i], "-n") == 0) {
            g_show_numbers = 1; arg_offset++;
        } else if (sc_strcmp(argv[i], "-i") == 0) {
            g_ignore_case = 1; arg_offset++;
        } else if (sc_strcmp(argv[i], "-c") == 0) {
            g_count_only = 1; arg_offset++;
        } else if (sc_strcmp(argv[i], "-v") == 0) {
            g_invert = 1; arg_offset++;
        } else if (sc_strcmp(argv[i], "-l") == 0) {
            g_files_only = 1; arg_offset++;
        } else if (sc_strcmp(argv[i], "-w") == 0) {
            g_word_match = 1; arg_offset++;
        } else if (sc_strcmp(argv[i], "-x") == 0) {
            g_line_match = 1; arg_offset++;
        } else if (sc_strcmp(argv[i], "-r") == 0 ||
                   sc_strcmp(argv[i], "-R") == 0) {
            g_recursive = 1; arg_offset++;
        } else if (sc_strcmp(argv[i], "-h") == 0 ||
                   sc_strcmp(argv[i], "--help") == 0) {
            print_usage();
            return 0;
        } else {
            printf("grep: unknown option '%s'\n", argv[i]);
            return 1;
        }
    }

    if (argc - arg_offset < 1) {
        print_usage();
        return 1;
    }

    g_pattern = argv[arg_offset];

    // Need at least a path when not reading stdin
    if (argc - arg_offset < 2) {
        print_usage();
        return 1;
    }

    // Multiple files → prefix output with filename
    g_multi_file = (argc - arg_offset > 2) || g_recursive;

    for (int i = arg_offset + 1; i < argc; i++) {
        if (g_recursive) {
            grep_recursive(argv[i]);
        } else {
            grep_file(argv[i]);
        }
    }

    return g_total_matches > 0 ? 0 : 1;
}
