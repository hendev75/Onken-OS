#include "math.h"
#include "errno.h"

__attribute__((weak)) double ldexp(double x, int expn) {
    double v = x;
    int i;
    if (expn >= 0) {
        for (i = 0; i < expn; i++) {
            v *= 2.0;
        }
    } else {
        for (i = 0; i < -expn; i++) {
            v *= 0.5;
        }
    }
    return v;
}

__attribute__((weak)) double frexp(double x, int *expn) {
    int e = 0;
    double v = x;
    if (x == 0.0) {
        *expn = 0;
        return 0.0;
    }
    while (fabs(v) >= 1.0) {
        v *= 0.5;
        e++;
    }
    while (fabs(v) > 0.0 && fabs(v) < 0.5) {
        v *= 2.0;
        e--;
    }
    *expn = e;
    return v;
}

static double _b_atan_series(double x) {
    double x2 = x * x;
    double term = x;
    double sum = x;
    int n;
    for (n = 3; n <= 23; n += 2) {
        term *= -x2;
        sum += term / (double)n;
    }
    return sum;
}

static double _b_atan_precise(double x) {
    if (x < 0.0) {
        return -_b_atan_precise(-x);
    }
    if (x > 1.0) {
        return (M_PI / 2.0) - _b_atan_precise(1.0 / x);
    }
    if (x > 0.5) {
        double y = (x - 1.0) / (x + 1.0);
        return (M_PI / 4.0) + _b_atan_series(y);
    }
    return _b_atan_series(x);
}

__attribute__((weak)) double atan2(double y, double x) {
    if (x > 0.0) {
        return _b_atan_precise(y / x);
    }
    if (x < 0.0) {
        if (y >= 0.0) return _b_atan_precise(y / x) + M_PI;
        return _b_atan_precise(y / x) - M_PI;
    }
    if (y > 0.0) return M_PI / 2.0;
    if (y < 0.0) return -M_PI / 2.0;
    return 0.0;
}

__attribute__((weak)) double asin(double x) {
    if (x > 1.0 || x < -1.0) {
        errno = EDOM;
        return 0.0;
    }
    return atan2(x, sqrt(1.0 - x * x));
}

__attribute__((weak)) double acos(double x) {
    if (x > 1.0 || x < -1.0) {
        errno = EDOM;
        return 0.0;
    }
    return M_PI / 2.0 - asin(x);
}
