#include "os.h"

int32_t power(int32_t base, int32_t n) {
    int32_t i, p;
    if (n == 0)
        return 1;
    p = 1;
    for (i = 1; i <= n; ++i)
        p = p * base;
    return p;
}

/**
 * @brief Return the absolute value of i.
 * 
 * @param i integral value
 * @return int32_t absolute value
 */
int32_t abs(int32_t i) {
    return i < 0 ? -i : i;
}

uint32_t alignUp(uint32_t val, uint32_t alignment) {
    if (!alignment)
        return val;
    --alignment;
    return (val + alignment) & ~alignment;
}

uint32_t alignDown(uint32_t val, uint32_t alignment) {
    if (!alignment)
        return val;
    return val & ~(alignment - 1);
}

uint32_t __attribute__((weak)) div64_32(uint64_t *n, uint32_t base) {
    uint64_t rem = *n;
    uint64_t b = base;
    uint64_t res, d = 1;
    uint32_t high = rem >> 32;

    /* Reduce the thing a bit first */
    res = 0;
    if (high >= base) {
        high /= base;
        res = (uint64_t)high << 32;
        rem -= (uint64_t)(high * base) << 32;
    }

    while ((int64_t)b > 0 && b < rem) {
        b = b + b;
        d = d + d;
    }

    do {
        if (rem >= b) {
            rem -= b;
            res += d;
        }
        b >>= 1;
        d >>= 1;
    } while (d);

    *n = res;
    return rem;
}
