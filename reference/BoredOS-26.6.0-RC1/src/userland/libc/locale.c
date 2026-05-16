#include "locale.h"
#include "string.h"

static struct lconv _b_lconv = {
    ".", "", "", "", "", ".", "", "", "", "",
    0, 0, 0, 0, 0, 0, 0, 0
};

__attribute__((weak)) char *setlocale(int category, const char *locale) {
    (void)category;
    if (locale == NULL || strcmp(locale, "C") == 0 || strcmp(locale, "") == 0) {
        return "C";
    }
    return NULL;
}

__attribute__((weak)) struct lconv *localeconv(void) {
    return &_b_lconv;
}
