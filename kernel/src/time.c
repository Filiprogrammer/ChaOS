#include "time.h"

#define CURRENT_YEAR 2019  // Change this each year!

uint8_t century_register = 0x00;  // Set by ACPI table parsing code if possible

uint8_t get_update_in_progress_flag() {
    return (cmos_read(0x0A) & 0x80);
}

/**
 * @brief Read the current time from the CMOS
 * 
 * @param ptm a pointer to a time_t struct that is to be filled with the current time and date
 */
void time_read_rtc(time_t *ptm) {
    uint8_t century = 20;
    uint8_t last_second;
    uint8_t last_minute;
    uint8_t last_hour;
    uint8_t last_day;
    uint8_t last_month;
    uint8_t last_year;
    uint8_t last_century;
    uint8_t registerB;

    // two register readings in a row have to be the same, to avoid getting inconsistent values due to RTC updates

    while (get_update_in_progress_flag());  // Make sure an update isn't in progress
    ptm->second = cmos_read(0x00);
    ptm->minute = cmos_read(0x02);
    ptm->hour = cmos_read(0x04);
    ptm->day = cmos_read(0x07);
    ptm->month = cmos_read(0x08);
    ptm->year = cmos_read(0x09);
    if (century_register)
        century = cmos_read(century_register);

    do {
        last_second = ptm->second;
        last_minute = ptm->minute;
        last_hour = ptm->hour;
        last_day = ptm->day;
        last_month = ptm->month;
        last_year = ptm->year;
        last_century = century;

        while (get_update_in_progress_flag());  // Make sure an update isn't in progress
        ptm->second = cmos_read(0x00);
        ptm->minute = cmos_read(0x02);
        ptm->hour = cmos_read(0x04);
        ptm->day = cmos_read(0x07);
        ptm->month = cmos_read(0x08);
        ptm->year = cmos_read(0x09);
        if (century_register)
            century = cmos_read(century_register);
    } while ((last_second != ptm->second) || (last_minute != ptm->minute) || (last_hour != ptm->hour) ||
             (last_day != ptm->day) || (last_month != ptm->month) || (last_year != ptm->year) ||
             (last_century != century));

    registerB = cmos_read(0x0B);

    // Convert BCD to binary values if necessary

    if (!(registerB & 0x04)) {
        ptm->second = (ptm->second & 0x0F) + ((ptm->second / 16) * 10);
        ptm->minute = (ptm->minute & 0x0F) + ((ptm->minute / 16) * 10);
        ptm->hour = ((ptm->hour & 0x0F) + (((ptm->hour & 0x70) / 16) * 10)) | (ptm->hour & 0x80);
        ptm->day = (ptm->day & 0x0F) + ((ptm->day / 16) * 10);
        ptm->month = (ptm->month & 0x0F) + ((ptm->month / 16) * 10);
        ptm->year = (ptm->year & 0x0F) + ((ptm->year / 16) * 10);
        if (century_register)
            century = (century & 0x0F) + ((century / 16) * 10);
    }

    // Convert 12 hour clock to 24 hour clock if necessary

    if (!(registerB & 0x02) && (ptm->hour & 0x80))
        ptm->hour = ((ptm->hour & 0x7F) + 12) % 24;

    // Calculate the full (4-digit) year

    if (century_register) {
        ptm->year += century * 100;
    } else {
        ptm->year += (CURRENT_YEAR / 100) * 100;
        if (ptm->year < CURRENT_YEAR) ptm->year += 100;
    }
}
