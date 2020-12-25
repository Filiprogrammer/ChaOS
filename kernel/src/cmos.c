#include "cmos.h"

/**
 * @brief Read byte from CMOS.
 * 
 * @param off offset in bytes
 * @return uint8_t byte corresponding to the given offset
 */
uint8_t cmos_read(uint8_t off) {
    uint8_t tmp = inportb(CMOS_ADDRESS);
    outportb(CMOS_ADDRESS, (tmp & 0x80) | (off & 0x7F));
    return inportb(CMOS_DATA);
}

/**
 * @brief Write byte to CMOS.
 * 
 * @param off offset in bytes
 * @param val byte to be written at the given offset
 */
void cmos_write(uint8_t off, uint8_t val) {
    uint8_t tmp = inportb(CMOS_ADDRESS);
    outportb(CMOS_ADDRESS, (tmp & 0x80) | (off & 0x7F));
    outportb(CMOS_DATA, val);
}
