#include "string.h"
#include <stdarg.h>

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

char* strncat(char* dest, const char* src, size_t n) {
    size_t dest_len = strlen(dest);
    size_t i;
    for (i = 0 ; i < n && src[i] != '\0' ; i++)
        dest[dest_len + i] = src[i];
    dest[dest_len + i] = '\0';
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

int strncmp(const char* s1, const char* s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) return 0;
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

int xsprintf(char* str, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int written = 0;
    while (*format) {
        if (*format == '%') {
            format++;
            int width = 0;
            int pad_zero = 0;
            int align_left = 0;
            if (*format == '-') {
                align_left = 1;
                format++;
            }
            if (*format == '0') {
                pad_zero = 1;
                format++;
            }
            while (*format >= '0' && *format <= '9') {
                width = width * 10 + (*format - '0');
                format++;
            }
            if (*format == 's') {
                char* s = va_arg(args, char*);
                int len = strlen(s);
                int pad = width - len;
                if (!align_left && pad > 0) {
                    while (pad--) {
                        *str++ = ' ';
                        written++;
                    }
                }
                while (*s) {
                    *str++ = *s++;
                    written++;
                }
                if (align_left && pad > 0) {
                    while (pad--) {
                        *str++ = ' ';
                        written++;
                    }
                }
            } else if (*format == 'd') {
                int val = va_arg(args, int);
                if (val < 0) {
                    *str++ = '-';
                    written++;
                    val = -val;
                }
                char num_buf[32];
                int i = 0;
                if (val == 0) {
                    num_buf[i++] = '0';
                } else {
                    while (val > 0) {
                        num_buf[i++] = (val % 10) + '0';
                        val /= 10;
                    }
                }
                int len = i;
                int pad = width - len;
                if (pad > 0) {
                    while (pad--) {
                        *str++ = pad_zero ? '0' : ' ';
                        written++;
                    }
                }
                while (i > 0) {
                    *str++ = num_buf[--i];
                    written++;
                }
            } else if (*format == 'c') {
                char c = (char)va_arg(args, int);
                *str++ = c;
                written++;
            } else {
                *str++ = '%';
                written++;
                *str++ = *format;
                written++;
            }
        } else {
            *str++ = *format;
            written++;
        }
        format++;
    }
    *str = '\0';
    va_end(args);
    return written;
}

