#ifndef MATH_H
#define MATH_H

#include "stdint.h"

#define NAN (__builtin_nanf(""))
#define INFINITY (__builtin_inff())
#define PI 3.14159265359

extern int32_t abs(int32_t i);
extern int32_t power(int32_t base,int32_t n);
extern uint32_t alignUp( uint32_t val, uint32_t alignment );
extern uint32_t alignDown( uint32_t val, uint32_t alignment );
extern uint32_t div64_32(uint64_t *n, uint32_t base);

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define SIGN(X) ((0 < (X)) - ((X) < 0))
#define CLAMP(X, Y, Z) MAX(Y, MIN(X, Z))

#endif
