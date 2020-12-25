#include "os.h"

int32_t ipow(int32_t base, int32_t exp) {
    int32_t result = 1;

    if (exp < 0) {
        if (base == 1)
            return 1;

        if (base == -1)
            return -1;

        return 0;
    }

    switch (31 - __builtin_clrsb(exp)) {
        case 255:
            if (base == 1)
                return 1;

            if (base == -1)
                return 1 - 2 * (exp & 1);

            // Return 0 on overflow/underflow
            return 0;
        case 5:
            if (exp & 1)
                result *= base;

            exp >>= 1;
            base *= base;
        case 4:
            if (exp & 1)
                result *= base;

            exp >>= 1;
            base *= base;
        case 3:
            if (exp & 1)
                result *= base;

            exp >>= 1;
            base *= base;
        case 2:
            if (exp & 1)
                result *= base;

            exp >>= 1;
            base *= base;
        case 1:
            if (exp & 1)
                result *= base;
        default:
            return result;
    }
}

double pow(double base, double exp) {
    double result = 0.0;

    __asm__ volatile(
        "fyl2x \n"
        "fld %%st(0) \n"
        "frndint \n"
        "fsubr %%st, %%st(1) \n"
        "fxch \n"
        "f2xm1 \n"
        "fld1 \n"
        "faddp %%st, %%st(1) \n"
        "fscale \n"
        : "=t"(result)
        : "0"(base), "u"(exp));

    return result;
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

/**
 * @brief Return the absolute value of i.
 * 
 * @param i integral value
 * @return int64_t absolute value
 */
int64_t llabs(int64_t i) {
    return i < 0 ? -i : i;
}

double fabs(double x) {
    *(int*)&x &= 0x7fffffff;
    return x;
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

uint8_t __ctzdi2(uint64_t val) {
    uint32_t val_high = val >> 32;
    uint32_t bitnr;
    __asm__("bsf %1, %0"
            : "=r"(bitnr)
            : "r"((uint32_t)val));

    if (val_high != 0 && ((uint32_t)val) == 0) {
        __asm__("bsf %1, %0"
                : "=r"(bitnr)
                : "r"(val_high));
        bitnr += 32;
    }

    return bitnr;
}

int32_t __clrsbsi2(int32_t x) {
    if (x & 0x80000000)
        x = ~x;

    if (x == 0)
        return 31;

    return __builtin_clz(x) - 1;
}

static inline uint64_t __attribute__((always_inline)) __udivdi3_inline(uint64_t dividend, uint64_t divisor) {
    const int power_of_two_factor = __builtin_ctzll(divisor);
    divisor >>= power_of_two_factor;
    dividend >>= power_of_two_factor;

    // Check for division by power of two or division by zero.
    if (divisor <= 1) {
        if (divisor == 0)
            // Division by zero
            return 0;

        return dividend;
    }

    const int divisor_clz = __builtin_clzll(divisor);
    const uint64_t max_divisor = divisor << divisor_clz;
    const uint64_t max_qbit = 1ULL << divisor_clz;

    uint64_t quotient = 0;
    uint64_t remainder = dividend;

    while (remainder >= divisor) {
        int shift = __builtin_clzll(remainder);
        uint64_t scaled_divisor = max_divisor >> shift;
        uint64_t quotient_bit = max_qbit >> shift;

        int too_big = (scaled_divisor > remainder);
        scaled_divisor >>= too_big;
        quotient_bit >>= too_big;
        remainder -= scaled_divisor;
        quotient |= quotient_bit;
    }

    return quotient;
}

uint64_t __udivdi3(uint64_t dividend, uint64_t divisor) {
    return __udivdi3_inline(dividend, divisor);
}

int64_t __divdi3(int64_t dividend, int64_t divisor) {
    uint64_t n = __udivdi3_inline(llabs(dividend), llabs(divisor));
    if ((dividend ^ divisor) < 0)
        n = -n;

    return (int64_t)n;
}

static inline uint64_t __attribute__((always_inline)) __umoddi3_inline(uint64_t dividend, uint64_t divisor) {
    // Shortcircuit mod by a power of two (and catch mod by zero).
    const uint64_t mask = divisor - 1;
    if ((divisor & mask) == 0) {
        if (divisor == 0)
            // Mod by zero
            return 0;

        return dividend & mask;
    }

    const uint64_t max_divisor = divisor << __builtin_clzll(divisor);

    uint64_t remainder = dividend;
    while (remainder >= divisor) {
        const int shift = __builtin_clzll(remainder);
        uint64_t scaled_divisor = max_divisor >> shift;
        scaled_divisor >>= (scaled_divisor > remainder);
        remainder -= scaled_divisor;
    }

    return remainder;
}

uint64_t __umoddi3(uint64_t dividend, uint64_t divisor) {
    return __umoddi3_inline(dividend, divisor);
}

int64_t __moddi3(int64_t dividend, int64_t divisor) {
    uint64_t remainder = __umoddi3_inline(llabs(dividend), llabs(divisor));
    return (int64_t)((dividend >= 0) ? remainder : -remainder);
}
