#include "errno.h"
#include "syscall.h"
#include "sys/stat.h"

static void _b_stat_init(struct stat *st) {
    st->st_dev = 0;
    st->st_ino = 0;
    st->st_mode = 0;
    st->st_nlink = 1;
    st->st_uid = 0;
    st->st_gid = 0;
    st->st_rdev = 0;
    st->st_size = 0;
    st->st_blksize = 512;
    st->st_blocks = 0;
    st->st_atime = 0;
    st->st_mtime = 0;
    st->st_ctime = 0;
}

__attribute__((weak)) int mkdir(const char *pathname, int mode) {
    (void)mode;
    if (!pathname || pathname[0] == '\0') {
        errno = EINVAL;
        return -1;
    }
    if (sys_exists(pathname)) {
        errno = EEXIST;
        return -1;
    }
    if (sys_mkdir(pathname) == 0) {
        return 0;
    }
    errno = EIO;
    return -1;
}

__attribute__((weak)) int access(const char *pathname, int mode) {
    (void)mode;
    if (!pathname || pathname[0] == '\0') {
        errno = EINVAL;
        return -1;
    }
    if (sys_exists(pathname)) {
        return 0;
    }
    errno = ENOENT;
    return -1;
}

__attribute__((weak)) int stat(const char *pathname, struct stat *statbuf) {
    FAT32_FileInfo info;
    if (!pathname || pathname[0] == '\0') {
        errno = EINVAL;
        return -1;
    }
    if (!sys_exists(pathname)) {
        errno = ENOENT;
        return -1;
    }

    if (statbuf) {
        _b_stat_init(statbuf);
        if (sys_get_file_info(pathname, &info) == 0) {
            statbuf->st_size = (int)info.size;
            if (info.is_directory) {
                statbuf->st_mode = S_IFDIR | 0755;
            } else {
                statbuf->st_mode = S_IFREG | 0644;
            }
            statbuf->st_blocks = (statbuf->st_size + 511) / 512;
        } else {
            int fd = sys_open(pathname, "rb");
            if (fd >= 0) {
                statbuf->st_size = (int)sys_size(fd);
                statbuf->st_mode = S_IFREG | 0644;
                statbuf->st_blocks = (statbuf->st_size + 511) / 512;
                sys_close(fd);
            } else {
                errno = EIO;
                return -1;
            }
        }
    }

    return 0;
}
