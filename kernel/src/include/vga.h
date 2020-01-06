#ifndef VGA_H
#define VGA_H

#include "os.h"

uint8_t* vga_set_mode(uint8_t mode);
void vga_update_cursor(uint16_t pos);

#endif
