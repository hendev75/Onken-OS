// Copyright (c) 2026 Lluciocc (https://github.com/lluciocc)
// This software is released under the GNU General Public License v3.0. See LICENSE file for details.
// This header needs to maintain in any file it is present in, as per the GPL license terms.
#include "../libc/syscall.h" // to fix Fat32_FileInfo definition
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdint.h"

#define TAR_BLOCK_SIZE 512
#define TAR_BUFFER_SIZE 4096
#define TAR_PATH_MAX 1024
#define TAR_LIST_ENTRIES 256

struct tar_header {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char padding[12];
} __attribute__((packed));

typedef char tar_header_size_must_be_512[(sizeof(struct tar_header) == TAR_BLOCK_SIZE) ? 1 : -1];

static void print_usage(void) {
    printf("Usage:\n");
    printf("  tar -cf archive.tar path...\n");
    printf("  tar -xf archive.tar\n");
    printf("  tar -tf archive.tar\n");
}

static int is_zero_block(const unsigned char *block) {
    for (int i = 0; i < TAR_BLOCK_SIZE; i++) {
        if (block[i] != 0) return 0;
    }
    return 1;
}

static int safe_copy(char *dst, int dst_size, const char *src) {
    int i = 0;

    if (dst_size <= 0) return -1;
    while (src[i]) {
        if (i >= dst_size - 1) return -1;
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
    return 0;
}

static int join_path(char *out, int out_size, const char *left, const char *right) {
    int len = 0;

    if (out_size <= 0) return -1;

    while (left[len]) {
        if (len >= out_size - 1) return -1;
        out[len] = left[len];
        len++;
    }

    if (len > 0 && out[len - 1] != '/') {
        if (len >= out_size - 1) return -1;
        out[len++] = '/';
    }

    for (int i = 0; right[i]; i++) {
        if (len >= out_size - 1) return -1;
        out[len++] = right[i];
    }

    out[len] = '\0';
    return 0;
}

static void strip_leading_slashes(const char **path) {
    while (**path == '/') {
        (*path)++;
    }
}

static int strip_trailing_slashes_copy(char *out, int out_size, const char *path) {
    int len;

    if (safe_copy(out, out_size, path) != 0) return -1;
    len = (int)strlen(out);
    while (len > 1 && out[len - 1] == '/') {
        out[--len] = '\0';
    }
    return 0;
}

static int has_unsafe_component(const char *path) {
    int start = 0;

    if (path[0] == '/') return 1;

    while (path[start]) {
        int end = start;
        while (path[end] && path[end] != '/') end++;

        if ((end - start) == 2 && path[start] == '.' && path[start + 1] == '.') {
            return 1;
        }

        start = end;
        while (path[start] == '/') start++;
    }

    return 0;
}

static int make_absolute_path(char *out, int out_size, const char *path) {
    char cwd[TAR_PATH_MAX];

    if (path[0] == '/') {
        return safe_copy(out, out_size, path);
    }

    if (sys_getcwd(cwd, sizeof(cwd)) < 0) {
        return -1;
    }

    return join_path(out, out_size, cwd, path);
}

static int make_directory_recursive(const char *path) {
    char tmp[TAR_PATH_MAX];
    int len = 0;

    if (!path || path[0] == '\0') return 0;
    if (make_absolute_path(tmp, sizeof(tmp), path) != 0) return -1;

    len = (int)strlen(tmp);
    while (len > 1 && tmp[len - 1] == '/') {
        tmp[--len] = '\0';
    }

    for (int i = 1; tmp[i]; i++) {
        if (tmp[i] == '/') {
            tmp[i] = '\0';
            if (tmp[0] != '\0' && !sys_exists(tmp)) {
                if (sys_mkdir(tmp) != 0 && !sys_exists(tmp)) return -1;
            }
            tmp[i] = '/';
        }
    }

    if (!sys_exists(tmp)) {
        if (sys_mkdir(tmp) != 0 && !sys_exists(tmp)) return -1;
    }

    return 0;
}

static int make_parent_directories(const char *path) {
    char parent[TAR_PATH_MAX];
    int last_slash = -1;

    if (safe_copy(parent, sizeof(parent), path) != 0) return -1;
    for (int i = 0; parent[i]; i++) {
        if (parent[i] == '/') last_slash = i;
    }

    if (last_slash <= 0) return 0;
    parent[last_slash] = '\0';
    return make_directory_recursive(parent);
}

static uint64_t octal_to_uint(const char *field, int size) {
    uint64_t value = 0;

    for (int i = 0; i < size; i++) {
        char c = field[i];
        if (c == '\0' || c == ' ') break;
        if (c < '0' || c > '7') continue;
        value = (value << 3) + (uint64_t)(c - '0');
    }

    return value;
}

static void uint_to_octal(uint64_t value, char *field, int size) {
    memset(field, '0', size);
    field[size - 1] = '\0';

    for (int i = size - 2; i >= 0; i--) {
        field[i] = (char)('0' + (value & 7));
        value >>= 3;
    }
}

static uint32_t calculate_checksum(struct tar_header *header) {
    unsigned char *bytes = (unsigned char *)header;
    uint32_t sum = 0;

    memset(header->checksum, ' ', sizeof(header->checksum));
    for (int i = 0; i < TAR_BLOCK_SIZE; i++) {
        sum += bytes[i];
    }
    return sum;
}

static void write_checksum(struct tar_header *header, uint32_t checksum) {
    for (int i = 0; i < 6; i++) {
        header->checksum[5 - i] = (char)('0' + (checksum & 7));
        checksum >>= 3;
    }
    header->checksum[6] = '\0';
    header->checksum[7] = ' ';
}

static int write_all(int fd, const void *buf, uint32_t len) {
    const char *p = (const char *)buf;
    uint32_t done = 0;

    while (done < len) {
        int written = sys_write_fs(fd, p + done, len - done);
        if (written <= 0) return -1;
        done += (uint32_t)written;
    }

    return 0;
}

static int read_exact(int fd, void *buf, uint32_t len) {
    char *p = (char *)buf;
    uint32_t done = 0;

    while (done < len) {
        int got = sys_read(fd, p + done, len - done);
        if (got <= 0) return -1;
        done += (uint32_t)got;
    }

    return 0;
}

static int write_padding(int fd, uint64_t size) {
    static const char zeros[TAR_BLOCK_SIZE] = {0};
    uint32_t pad = (uint32_t)((TAR_BLOCK_SIZE - (size % TAR_BLOCK_SIZE)) % TAR_BLOCK_SIZE);

    if (pad == 0) return 0;
    return write_all(fd, zeros, pad);
}

static int split_ustar_path(const char *path, char *name, char *prefix) {
    int len = (int)strlen(path);

    memset(name, 0, 100);
    memset(prefix, 0, 155);

    if (len <= 100) {
        memcpy(name, path, len);
        return 0;
    }

    for (int i = len - 1; i > 0; i--) {
        int prefix_len;
        int name_len;

        if (path[i] != '/') continue;

        prefix_len = i;
        name_len = len - i - 1;
        if (prefix_len <= 155 && name_len > 0 && name_len <= 100) {
            memcpy(prefix, path, prefix_len);
            memcpy(name, path + i + 1, name_len);
            return 0;
        }
    }

    return -1;
}

static int write_header(int archive_fd, const char *archive_path, uint64_t size, char typeflag) {
    struct tar_header header;
    uint32_t checksum;

    memset(&header, 0, sizeof(header));

    if (split_ustar_path(archive_path, header.name, header.prefix) != 0) {
        printf("tar: path too long for ustar: %s\n", archive_path);
        return -1;
    }

    uint_to_octal(typeflag == '5' ? 0755 : 0644, header.mode, sizeof(header.mode));
    uint_to_octal(0, header.uid, sizeof(header.uid));
    uint_to_octal(0, header.gid, sizeof(header.gid));
    uint_to_octal(typeflag == '5' ? 0 : size, header.size, sizeof(header.size));
    uint_to_octal(0, header.mtime, sizeof(header.mtime));
    header.typeflag = typeflag;
    memcpy(header.magic, "ustar", 5);
    memcpy(header.version, "00", 2);

    checksum = calculate_checksum(&header);
    write_checksum(&header, checksum);

    return write_all(archive_fd, &header, sizeof(header));
}

static int make_archive_path(char *out, int out_size, const char *input_path, int is_directory) {
    char tmp[TAR_PATH_MAX];
    const char *p = input_path;
    int len;

    strip_leading_slashes(&p);
    if (*p == '\0') {
        printf("tar: refusing to archive root path '%s'\n", input_path);
        return -1;
    }

    if (safe_copy(tmp, sizeof(tmp), p) != 0) return -1;
    len = (int)strlen(tmp);
    while (len > 1 && tmp[len - 1] == '/') {
        tmp[--len] = '\0';
    }

    if (is_directory) {
        if (len + 1 >= out_size) return -1;
        memcpy(out, tmp, len);
        out[len++] = '/';
        out[len] = '\0';
    } else {
        if (safe_copy(out, out_size, tmp) != 0) return -1;
    }

    return 0;
}

static int add_file_to_tar(int archive_fd, const char *fs_path, const char *archive_path) {
    FAT32_FileInfo info;
    int input_fd;
    char buffer[TAR_BUFFER_SIZE];
    uint64_t remaining;

    if (sys_get_file_info(fs_path, &info) != 0 || info.is_directory) {
        printf("tar: cannot stat file '%s'\n", fs_path);
        return -1;
    }

    input_fd = sys_open(fs_path, "r");
    if (input_fd < 0) {
        printf("tar: cannot open '%s'\n", fs_path);
        return -1;
    }

    if (write_header(archive_fd, archive_path, info.size, '0') != 0) {
        sys_close(input_fd);
        return -1;
    }

    remaining = info.size;
    while (remaining > 0) {
        uint32_t chunk = remaining > TAR_BUFFER_SIZE ? TAR_BUFFER_SIZE : (uint32_t)remaining;
        int got = sys_read(input_fd, buffer, chunk);
        if (got <= 0) {
            printf("tar: read error on '%s'\n", fs_path);
            sys_close(input_fd);
            return -1;
        }
        if (write_all(archive_fd, buffer, (uint32_t)got) != 0) {
            printf("tar: write error on archive while adding '%s'\n", fs_path);
            sys_close(input_fd);
            return -1;
        }
        remaining -= (uint32_t)got;
    }

    sys_close(input_fd);

    /* File data is padded so the next header begins on a 512-byte boundary. */
    if (write_padding(archive_fd, info.size) != 0) {
        printf("tar: write error on archive padding\n");
        return -1;
    }

    return 0;
}

static int add_directory_recursive(int archive_fd, const char *fs_path, const char *archive_path) {
    FAT32_FileInfo *entries;
    int count;
    int had_error = 0;

    if (write_header(archive_fd, archive_path, 0, '5') != 0) return -1;

    entries = (FAT32_FileInfo *)malloc(sizeof(FAT32_FileInfo) * TAR_LIST_ENTRIES);
    if (!entries) {
        printf("tar: out of memory reading directory '%s'\n", fs_path);
        return -1;
    }

    count = sys_list(fs_path, entries, TAR_LIST_ENTRIES);
    if (count < 0) {
        printf("tar: cannot read directory '%s'\n", fs_path);
        free(entries);
        return -1;
    }

    for (int i = 0; i < count; i++) {
        char child_fs[TAR_PATH_MAX];
        char child_archive[TAR_PATH_MAX];
        int child_len;

        if (strcmp(entries[i].name, ".") == 0 || strcmp(entries[i].name, "..") == 0) {
            continue;
        }

        if (join_path(child_fs, sizeof(child_fs), fs_path, entries[i].name) != 0) {
            printf("tar: path too long below '%s'\n", fs_path);
            had_error = 1;
            continue;
        }

        if (join_path(child_archive, sizeof(child_archive), archive_path, entries[i].name) != 0) {
            printf("tar: archive path too long below '%s'\n", archive_path);
            had_error = 1;
            continue;
        }

        child_len = (int)strlen(child_archive);
        if (entries[i].is_directory) {
            if (child_len + 1 >= (int)sizeof(child_archive)) {
                printf("tar: archive path too long below '%s'\n", archive_path);
                had_error = 1;
                continue;
            }
            child_archive[child_len++] = '/';
            child_archive[child_len] = '\0';
            if (add_directory_recursive(archive_fd, child_fs, child_archive) != 0) had_error = 1;
        } else {
            if (add_file_to_tar(archive_fd, child_fs, child_archive) != 0) had_error = 1;
        }
    }

    free(entries);
    return had_error ? -1 : 0;
}

static int add_path_to_tar(int archive_fd, const char *path) {
    FAT32_FileInfo info;
    char fs_path[TAR_PATH_MAX];
    char archive_path[TAR_PATH_MAX];

    if (strip_trailing_slashes_copy(fs_path, sizeof(fs_path), path) != 0) {
        printf("tar: path too long: %s\n", path);
        return -1;
    }

    if (sys_get_file_info(fs_path, &info) != 0) {
        printf("tar: cannot stat '%s'\n", path);
        return -1;
    }

    if (make_archive_path(archive_path, sizeof(archive_path), fs_path, info.is_directory) != 0) {
        printf("tar: cannot store path '%s'\n", path);
        return -1;
    }

    if (info.is_directory) {
        return add_directory_recursive(archive_fd, fs_path, archive_path);
    }

    return add_file_to_tar(archive_fd, fs_path, archive_path);
}

static int build_full_name(const struct tar_header *header, char *out, int out_size) {
    int pos = 0;

    if (out_size <= 0) return -1;

    if (header->prefix[0]) {
        for (int i = 0; i < 155 && header->prefix[i]; i++) {
            if (pos >= out_size - 1) return -1;
            out[pos++] = header->prefix[i];
        }
        if (pos >= out_size - 1) return -1;
        out[pos++] = '/';
    }

    for (int i = 0; i < 100 && header->name[i]; i++) {
        if (pos >= out_size - 1) return -1;
        out[pos++] = header->name[i];
    }

    out[pos] = '\0';
    return pos > 0 ? 0 : -1;
}

static int valid_ustar_magic(const struct tar_header *header) {
    return memcmp(header->magic, "ustar", 5) == 0;
}

static int skip_bytes(int fd, uint64_t size) {
    char buffer[TAR_BUFFER_SIZE];
    uint64_t remaining = size;

    while (remaining > 0) {
        uint32_t chunk = remaining > TAR_BUFFER_SIZE ? TAR_BUFFER_SIZE : (uint32_t)remaining;
        int got = sys_read(fd, buffer, chunk);
        if (got <= 0) return -1;
        remaining -= (uint32_t)got;
    }

    return 0;
}

static int skip_entry_data(int fd, uint64_t size) {
    uint64_t total = size + ((TAR_BLOCK_SIZE - (size % TAR_BLOCK_SIZE)) % TAR_BLOCK_SIZE);
    return skip_bytes(fd, total);
}

static int extract_entry(int archive_fd, const struct tar_header *header, const char *path, uint64_t size) {
    char buffer[TAR_BUFFER_SIZE];
    uint64_t remaining;
    int out_fd;

    if (has_unsafe_component(path)) {
        printf("tar: skipping unsafe path '%s'\n", path);
        return skip_entry_data(archive_fd, size);
    }

    if (header->typeflag == '5') {
        if (make_directory_recursive(path) != 0) {
            printf("tar: cannot create directory '%s'\n", path);
            skip_entry_data(archive_fd, size);
            return -1;
        }
        return skip_entry_data(archive_fd, size);
    }

    if (header->typeflag != '0' && header->typeflag != '\0') {
        printf("tar: skipping unsupported entry '%s'\n", path);
        return skip_entry_data(archive_fd, size);
    }

    if (make_parent_directories(path) != 0) {
        printf("tar: cannot create parent directories for '%s'\n", path);
        return skip_entry_data(archive_fd, size);
    }

    out_fd = sys_open(path, "w");
    if (out_fd < 0) {
        printf("tar: cannot create '%s'\n", path);
        return skip_entry_data(archive_fd, size);
    }

    remaining = size;
    while (remaining > 0) {
        uint32_t chunk = remaining > TAR_BUFFER_SIZE ? TAR_BUFFER_SIZE : (uint32_t)remaining;
        int got = sys_read(archive_fd, buffer, chunk);
        if (got <= 0) {
            printf("tar: unexpected end of archive while reading '%s'\n", path);
            sys_close(out_fd);
            return -1;
        }
        if (write_all(out_fd, buffer, (uint32_t)got) != 0) {
            printf("tar: write error on '%s'\n", path);
            sys_close(out_fd);
            skip_entry_data(archive_fd, remaining - (uint32_t)got);
            return -1;
        }
        remaining -= (uint32_t)got;
    }

    sys_close(out_fd);
    return skip_bytes(archive_fd, (TAR_BLOCK_SIZE - (size % TAR_BLOCK_SIZE)) % TAR_BLOCK_SIZE);
}

static int create_archive(const char *archive_name, int path_count, char **paths) {
    static const char zero_block[TAR_BLOCK_SIZE] = {0};
    int archive_fd;
    int had_error = 0;

    archive_fd = sys_open(archive_name, "w");
    if (archive_fd < 0) {
        printf("tar: cannot create '%s'\n", archive_name);
        return 1;
    }

    for (int i = 0; i < path_count; i++) {
        if (add_path_to_tar(archive_fd, paths[i]) != 0) had_error = 1;
    }

    /* Two zero blocks mark end-of-archive for standard tar readers. */
    if (write_all(archive_fd, zero_block, TAR_BLOCK_SIZE) != 0 ||
        write_all(archive_fd, zero_block, TAR_BLOCK_SIZE) != 0) {
        printf("tar: write error on '%s'\n", archive_name);
        had_error = 1;
    }

    sys_close(archive_fd);
    return had_error ? 1 : 0;
}

static int read_archive(const char *archive_name, int extract) {
    int archive_fd;
    int had_error = 0;

    archive_fd = sys_open(archive_name, "r");
    if (archive_fd < 0) {
        printf("tar: cannot open '%s'\n", archive_name);
        return 1;
    }

    while (1) {
        struct tar_header header;
        char path[TAR_PATH_MAX];
        uint64_t size;

        if (read_exact(archive_fd, &header, TAR_BLOCK_SIZE) != 0) {
            printf("tar: unexpected end of archive\n");
            had_error = 1;
            break;
        }

        if (is_zero_block((const unsigned char *)&header)) {
            break;
        }

        if (!valid_ustar_magic(&header)) {
            printf("tar: invalid or unsupported archive format\n");
            had_error = 1;
            break;
        }

        if (build_full_name(&header, path, sizeof(path)) != 0) {
            printf("tar: entry path too long\n");
            had_error = 1;
            break;
        }

        size = octal_to_uint(header.size, sizeof(header.size));

        if (extract) {
            if (extract_entry(archive_fd, &header, path, size) != 0) had_error = 1;
        } else {
            printf("%s\n", path);
            if (skip_entry_data(archive_fd, size) != 0) {
                printf("tar: unexpected end of archive\n");
                had_error = 1;
                break;
            }
        }
    }

    sys_close(archive_fd);
    return had_error ? 1 : 0;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        print_usage();
        return 1;
    }

    if (strcmp(argv[1], "-cf") == 0) {
        if (argc < 4) {
            print_usage();
            return 1;
        }
        return create_archive(argv[2], argc - 3, &argv[3]);
    }

    if (strcmp(argv[1], "-xf") == 0) {
        if (argc != 3) {
            print_usage();
            return 1;
        }
        return read_archive(argv[2], 1);
    }

    if (strcmp(argv[1], "-tf") == 0) {
        if (argc != 3) {
            print_usage();
            return 1;
        }
        return read_archive(argv[2], 0);
    }

    print_usage();
    return 1;
}
