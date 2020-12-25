#ifndef TIME_H
#define TIME_H

#include "cmos.h"
#include "os.h"

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
