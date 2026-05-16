// Copyright (c) 2023-2026 Christiaan (chris@boreddev.nl)
// This software is released under the GNU General Public License v3.0. See LICENSE file for details.
// This header needs to maintain in any file it is present in, as per the GPL license terms.
#include <stdlib.h>
#include <syscall.h>
#include <stdbool.h>

static void str_copy(char *dst, const char *src, int max_len) {
    int i = 0;
    if (!dst || max_len <= 0) return;
    if (!src) { dst[0] = 0; return; }
    while (i < max_len - 1 && src[i]) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = 0;
}

static bool starts_with(const char *s, const char *prefix) {
    if (!s || !prefix) return false;
    while (*prefix) {
        if (*s != *prefix) return false;
        s++; prefix++;
    }
    return true;
}

static void trim_end(char *s) {
    int len;
    if (!s) return;
    len = (int)strlen(s);
    while (len > 0) {
        char c = s[len - 1];
        if (c == '\n' || c == '\r' || c == ' ' || c == '\t') {
            s[len - 1] = 0;
            len--;
        } else {
            break;
        }
    }
}

static const char *trim_start(const char *s) {
    if (!s) return "";
    while (*s == ' ' || *s == '\t') s++;
    return s;
}

static void copy_range(char *dst, const char *start, int len, int max_len) {
    int i;
    if (!dst || max_len <= 0) return;
    if (!start || len <= 0) { dst[0] = 0; return; }
    if (len > max_len - 1) len = max_len - 1;
    for (i = 0; i < len; i++) dst[i] = start[i];
    dst[len] = 0;
    trim_end(dst);
}

static void copy_line(const char *start, char *out, int out_len) {
    int i = 0;
    if (!start || !out || out_len <= 0) return;
    while (start[i] && start[i] != '\n' && start[i] != '\r' && i < out_len - 1) {
        out[i] = start[i];
        i++;
    }
    out[i] = 0;
    trim_end(out);
}

static int read_file_to_buf(const char *path, char *buf, int max_len) {
    int fd;
    int bytes;
    if (!buf || max_len <= 0) return -1;
    fd = sys_open(path, "r");
    if (fd < 0) return -1;
    bytes = sys_read(fd, buf, max_len - 1);
    sys_close(fd);
    if (bytes < 0) return -1;
    buf[bytes] = 0;
    return bytes;
}

static void parse_version_info(const char *vbuf,
                               char *os_name, int os_name_len,
                               char *kernel_name, int kernel_name_len,
                               char *kernel_ver, int kernel_ver_len,
                               char *build_str, int build_str_len) {
    char line1[256] = {0};
    char line2[256] = {0};
    char line3[256] = {0};
    const char *l2;
    const char *l3;
    const char *end;

    if (!vbuf || !vbuf[0]) return;

    copy_line(vbuf, line1, sizeof(line1));
    l2 = strchr(vbuf, '\n');
    if (l2) {
        l2++;
        copy_line(l2, line2, sizeof(line2));
        l3 = strchr(l2, '\n');
        if (l3) {
            l3++;
            copy_line(l3, line3, sizeof(line3));
        }
    }

    if (line1[0]) {
        end = strstr(line1, " [");
        if (!end) end = strstr(line1, " Version");
        if (!end) end = line1 + (int)strlen(line1);
        copy_range(os_name, line1, (int)(end - line1), os_name_len);
    }

    if (line2[0]) {
        const char *p = line2;
        if (starts_with(p, "Kernel:")) {
            p += 7;
            if (*p == ' ') p++;
        }
        p = trim_start(p);
        if (*p) {
            const char *t = p;
            while (*t && *t != ' ') t++;
            copy_range(kernel_name, p, (int)(t - p), kernel_name_len);
            while (*t == ' ') t++;
            if (*t) copy_range(kernel_ver, t, (int)strlen(t), kernel_ver_len);
        }
    }

    if (line3[0]) {
        const char *p = line3;
        if (starts_with(p, "Build:")) {
            p += 6;
            if (*p == ' ') p++;
        }
        p = trim_start(p);
        if (*p) copy_range(build_str, p, (int)strlen(p), build_str_len);
    }
}

static void parse_cpu_model(const char *buf, char *out, int out_len) {
    const char *p;
    const char *colon;
    const char *end;
    if (!buf || !out || out_len <= 0) return;
    p = strstr(buf, "model name");
    if (!p) return;
    colon = strchr(p, ':');
    if (!colon) return;
    colon++;
    colon = trim_start(colon);
    end = colon;
    while (*end && *end != '\n' && *end != '\r') end++;
    copy_range(out, colon, (int)(end - colon), out_len);
}

static void print_usage(void) {
    printf("Usage: uname [-amnoprsv]\n");
}

static void print_field(const char *value, bool *first) {
    if (!value || !value[0]) value = "Unknown";
    if (!*first) printf(" ");
    printf("%s", value);
    *first = false;
}

int main(int argc, char **argv) {
    bool want_s = false;
    bool want_n = false;
    bool want_r = false;
    bool want_v = false;
    bool want_m = false;
    bool want_p = false;
    bool want_o = false;
    bool any_flags = false;

    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];
        if (!arg || arg[0] != '-' || arg[1] == 0) {
            print_usage();
            return 1;
        }
        for (int j = 1; arg[j]; j++) {
            char c = arg[j];
            if (c == 'a') {
                want_s = want_n = want_r = want_v = want_m = want_p = want_o = true;
                any_flags = true;
            } else if (c == 's') { want_s = true; any_flags = true; }
            else if (c == 'n') { want_n = true; any_flags = true; }
            else if (c == 'r') { want_r = true; any_flags = true; }
            else if (c == 'v') { want_v = true; any_flags = true; }
            else if (c == 'm') { want_m = true; any_flags = true; }
            else if (c == 'p') { want_p = true; any_flags = true; }
            else if (c == 'o') { want_o = true; any_flags = true; }
            else { print_usage(); return 1; }
        }
    }

    if (!any_flags) want_s = true;

    char vbuf[1024] = {0};
    char cpubuf[2048] = {0};
    char os_name[64] = "Unknown";
    char kernel_name[64] = "Unknown";
    char kernel_ver[64] = "Unknown";
    char build_str[64] = "Unknown";
    char processor[128] = {0};

    if (read_file_to_buf("/proc/version", vbuf, sizeof(vbuf)) > 0) {
        parse_version_info(vbuf, os_name, sizeof(os_name), kernel_name, sizeof(kernel_name),
                           kernel_ver, sizeof(kernel_ver), build_str, sizeof(build_str));
    }

    if (read_file_to_buf("/proc/cpuinfo", cpubuf, sizeof(cpubuf)) > 0) {
        parse_cpu_model(cpubuf, processor, sizeof(processor));
    }

    const char *nodename = "boredos";
    const char *machine = "x86_64";
    if (!processor[0]) str_copy(processor, machine, sizeof(processor));

    bool first = true;
    if (want_s) print_field(kernel_name, &first);
    if (want_n) print_field(nodename, &first);
    if (want_r) print_field(kernel_ver, &first);
    if (want_v) print_field(build_str, &first);
    if (want_m) print_field(machine, &first);
    if (want_p) print_field(processor, &first);
    if (want_o) print_field(os_name, &first);
    printf("\n");

    return 0;
}
