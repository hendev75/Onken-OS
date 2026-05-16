#include "string.h"
#include "errno.h"
#include "ctype.h"
#include "stdlib.h"

__attribute__((weak)) int strncmp(const char *s1, const char *s2, size_t n) {
    size_t i;
    for (i = 0; i < n; i++) {
        unsigned char c1 = (unsigned char)s1[i];
        unsigned char c2 = (unsigned char)s2[i];
        if (c1 != c2) {
            return (int)c1 - (int)c2;
        }
        if (c1 == '\0') {
            return 0;
        }
    }
    return 0;
}

__attribute__((weak)) char *strncpy(char *dest, const char *src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}

__attribute__((weak)) char *strncat(char *dest, const char *src, size_t n) {
    size_t dlen = strlen(dest);
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[dlen + i] = src[i];
    }
    dest[dlen + i] = '\0';
    return dest;
}

__attribute__((weak)) char *strrchr(const char *s, int c) {
    const char *last = NULL;
    for (;; s++) {
        if (*s == (char)c) {
            last = s;
        }
        if (*s == '\0') {
            break;
        }
    }
    return (char *)last;
}

__attribute__((weak)) char *strpbrk(const char *s, const char *accept) {
    for (; *s; s++) {
        const char *a;
        for (a = accept; *a; a++) {
            if (*s == *a) {
                return (char *)s;
            }
        }
    }
    return NULL;
}

__attribute__((weak)) size_t strspn(const char *s, const char *accept) {
    size_t n = 0;
    while (*s) {
        if (!strchr(accept, *s)) {
            break;
        }
        n++;
        s++;
    }
    return n;
}

__attribute__((weak)) size_t strcspn(const char *s, const char *reject) {
    size_t n = 0;
    while (*s) {
        if (strchr(reject, *s)) {
            break;
        }
        n++;
        s++;
    }
    return n;
}

__attribute__((weak)) void *memchr(const void *s, int c, size_t n) {
    const unsigned char *p = (const unsigned char *)s;
    size_t i;
    for (i = 0; i < n; i++) {
        if (p[i] == (unsigned char)c) {
            return (void *)(p + i);
        }
    }
    return NULL;
}

__attribute__((weak)) int strcoll(const char *s1, const char *s2) {
    return strcmp(s1, s2);
}

__attribute__((weak)) char *strerror(int errnum) {
    switch (errnum) {
        case 0: return "no error";
        case EDOM: return "domain error";
        case ERANGE: return "range error";
        case EINVAL: return "invalid argument";
        case ENOENT: return "no such file or directory";
        case ENOMEM: return "not enough memory";
        case EEXIST: return "file exists";
        case EIO: return "i/o error";
        case ENOSYS: return "function not implemented";
        default: return "unknown error";
    }
}

__attribute__((weak)) int strncasecmp(const char *s1, const char *s2, size_t n) {
    while (n--) {
        char c1 = (char)tolower((unsigned char)*s1++);
        char c2 = (char)tolower((unsigned char)*s2++);
        if (c1 != c2) {
            return c1 - c2;
        }
        if (!c1) {
            break;
        }
    }
    return 0;
}

__attribute__((weak)) int strcasecmp(const char *s1, const char *s2) {
    while (1) {
        char c1 = (char)tolower((unsigned char)*s1++);
        char c2 = (char)tolower((unsigned char)*s2++);
        if (c1 != c2) {
            return c1 - c2;
        }
        if (!c1) {
            break;
        }
    }
    return 0;
}

__attribute__((weak)) char *strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *dup = (char *)malloc(len);
    if (!dup) {
        errno = ENOMEM;
        return NULL;
    }
    memcpy(dup, s, len);
    return dup;
}
