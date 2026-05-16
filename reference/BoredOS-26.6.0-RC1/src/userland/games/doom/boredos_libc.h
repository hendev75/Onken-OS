#ifndef BOREDOS_LIBC_H
#define BOREDOS_LIBC_H

#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define EOF (-1)

extern FILE* stderr;
extern FILE* stdout;
extern FILE* stdin;

#define O_RDONLY 1
#define O_WRONLY 2
#define O_RDWR   3
#define O_CREAT  4
#define O_TRUNC  8
#define O_BINARY 0

#define F_OK 0
#define R_OK 4
#define W_OK 2
#define X_OK 1

int open(const char *pathname, int flags, ...);
int close(int fd);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
off_t lseek(int fd, off_t offset, int whence);
int unlink(const char *pathname);
int isatty(int fd);

FILE *fopen(const char *path, const char *mode);
int fclose(FILE *stream);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
int fseek(FILE *stream, long offset, int whence);
long ftell(FILE *stream);
int remove(const char *pathname);
int rename(const char *oldpath, const char *newpath);
int fflush(FILE *stream);
int fprintf(FILE *stream, const char *format, ...);
int sprintf(char *str, const char *format, ...);
int snprintf(char *str, size_t size, const char *format, ...);
int vsnprintf(char *str, size_t size, const char *format, va_list ap);
int sscanf(const char *str, const char *format, ...);
int fputc(int c, FILE *stream);
int fputs(const char *s, FILE *stream);
long filelength(FILE *f);

int strncasecmp(const char *s1, const char *s2, size_t n);
int strcasecmp(const char *s1, const char *s2);
char *strncpy(char *dest, const char *src, size_t n);
int strncmp(const char *s1, const char *s2, size_t n);
char *strrchr(const char *s, int c);
char *strchr(const char *s, int c);
char *strdup(const char *s);

int toupper(int c);
int tolower(int c);
int isspace(int c);
int isdigit(int c);
int isprint(int c);
int isalpha(int c);
int isalnum(int c);
int isgraph(int c);
int ispunct(int c);
int isupper(int c);

int mkdir(const char *pathname, int mode);
int access(const char *pathname, int mode);
int stat(const char *pathname, struct stat *statbuf);

char *strstr(const char *haystack, const char *needle);
extern int errno;

#define M_PI 3.14159265358979323846

void _exit(int status);
void _exit(int status);
void *memmove(void *dest, const void *src, size_t n);
int abs(int j);
int putchar(int c);
int system(const char *command);
int vfprintf(FILE *stream, const char *format, va_list ap);

#endif
