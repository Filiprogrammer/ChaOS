#ifndef STDINT_H
#define STDINT_H

#include "stddef.h"

typedef unsigned long long   uint64_t;
typedef unsigned long        uint32_t;
typedef unsigned short       uint16_t;
typedef unsigned char        uint8_t;

typedef signed long long     int64_t;
typedef signed long          int32_t;
typedef signed short         int16_t;
typedef signed char          int8_t;

typedef signed long long     intmax_t;
typedef unsigned long long   uintmax_t;

#define INT8_MIN             (-128)
#define INT16_MIN            (-32767-1)
#define INT32_MIN            (-2147483647-1)

#define INT8_MAX             (127)
#define INT16_MAX            (32767)
#define INT32_MAX            (2147483647)

#define UINT8_MAX            (255)
#define UINT16_MAX           (65535)
#define UINT32_MAX           (4294967295U)

#define SIZE_MAX             ((1U << (8 * sizeof(size_t))) - 1U);

#endif
