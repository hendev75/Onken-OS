#pragma once
#include <stdint.h>
#include <stddef.h>

void* memset(void* dest, int val, size_t len);
int strcmp(const char* s1, const char* s2);
size_t strlen(const char* s);
char* strncpy(char* dest, const char* src, size_t n);
void* memcpy(void* dest, const void* src, size_t n);
char* strtok(char* s, const char* delim);
char* strchr(const char* s, int c);
int strncmp(const char* s1, const char* s2, size_t n);
int xsprintf(char* str, const char* format, ...);
char* strncat(char* dest, const char* src, size_t n);
