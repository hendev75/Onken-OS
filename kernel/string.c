#include "string.h"

void* memset(void* dest, int val, size_t len) {
    uint8_t* ptr = (uint8_t*)dest;
    while (len-- > 0)
        *ptr++ = val;
    return dest;
}

int strcmp(const char* s1, const char* s2) {
    while(*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

size_t strlen(const char* s) {
    size_t len = 0;
    while(s[len]) len++;
    return len;
}

void* memcpy(void* dest, const void* src, size_t n) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    while (n--) *d++ = *s++;
    return dest;
}

char* strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++)
        dest[i] = src[i];
    for ( ; i < n; i++)
        dest[i] = '\0';
    return dest;
}

static char* last_strtok = NULL;
char* strtok(char* s, const char* delim) {
    if (s == NULL) s = last_strtok;
    if (s == NULL) return NULL;
    
    // Skip delimiters
    while (*s && strchr(delim, *s)) s++;
    if (*s == '\0') return last_strtok = NULL;
    
    char* start = s;
    while (*s && !strchr(delim, *s)) s++;
    if (*s) {
        *s = '\0';
        last_strtok = s + 1;
    } else {
        last_strtok = NULL;
    }
    return start;
}

char* strchr(const char* s, int c) {
    while (*s) {
        if (*s == (char)c) return (char*)s;
        s++;
    }
    return NULL;
}
