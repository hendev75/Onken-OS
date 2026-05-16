#ifndef DOOM_COMPAT_SYS_STAT_H
#define DOOM_COMPAT_SYS_STAT_H

#include "types.h"

struct stat {
	int st_size;
	int st_mode;
};

int stat(const char *pathname, struct stat *statbuf);
int fstat(int fd, struct stat *statbuf);
int mkdir(const char *pathname, int mode);

#endif
