#ifndef TIME_H
#define TIME_H

#include "os.h"
#include "cmos.h"

#define CMOS_ADDRESS 0x70
#define CMOS_DATA 0x71

typedef struct {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} time_t;

void time_read_rtc(time_t *ptm);

#endif
