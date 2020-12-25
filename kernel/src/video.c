#include "math.h"
#include "os.h"
#include "stdarg.h"
#include "string.h"
#include "vga.h"

// Video modes
// 0    Text        80x50x4
// 1    Text        80x25x4
// 2    Graphics    320x200x8

// 32 bit color format
// aaaaaaaarrrrrrrrggggggggbbbbbbbb

uint8_t video_mode = 0;
uint16_t csr_x = 0;
uint16_t csr_y = 0;
uint8_t txtcolor = 0x0F;
uint32_t color = 0xFFFFFFFF;
uint16_t width = 80;
uint16_t height = 50;
uint8_t saved_csr_x = 0;
uint8_t saved_csr_y = 0;

uint8_t* vidmem = (uint8_t*)0xB8000;

const uint8_t vga_font_8x16[] = {
    0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x24, 0x24, 0x7E, 0x24, 0x24, 0x7E, 0x24, 0x24, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x18, 0x7E, 0x60, 0x7E, 0x06, 0x06, 0x7E, 0x18, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x64, 0x64, 0x68, 0x08, 0x10, 0x16, 0x26, 0x26, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x3C, 0x6C, 0x60, 0x3E, 0x6C, 0x6C, 0x6C, 0x3E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x18, 0x0C, 0x00, 0x00,
    0x00, 0x00, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x18, 0x30, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x18, 0x5A, 0x7E, 0x18, 0x7E, 0x5A, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x7E, 0x7E, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x08, 0x38, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x06, 0x06, 0x0C, 0x0C, 0x18, 0x18, 0x30, 0x30, 0x60, 0x60, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x18, 0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x7C, 0x46, 0x06, 0x3C, 0x60, 0x60, 0x62, 0x7E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x7C, 0x46, 0x06, 0x3C, 0x06, 0x06, 0x46, 0x7C, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x0C, 0x1C, 0x3C, 0x6C, 0x4C, 0x7E, 0x0C, 0x0C, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x7E, 0x62, 0x60, 0x7C, 0x06, 0x06, 0x46, 0x7C, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x3E, 0x62, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x3C, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x7E, 0x46, 0x06, 0x0C, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x66, 0x3C, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x3E, 0x06, 0x46, 0x7C, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x08, 0x38, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x1E, 0x78, 0x1E, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x78, 0x1E, 0x78, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x7E, 0x66, 0x06, 0x1E, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x3C, 0x42, 0x5A, 0x5A, 0x5E, 0x40, 0x7E, 0x3E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x18, 0x3C, 0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x7E, 0x66, 0x6C, 0x78, 0x6C, 0x66, 0x66, 0x7E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x7E, 0x66, 0x60, 0x60, 0x60, 0x60, 0x66, 0x7E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x78, 0x6C, 0x66, 0x66, 0x66, 0x66, 0x6C, 0x78, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x7E, 0x60, 0x60, 0x7E, 0x60, 0x60, 0x60, 0x7E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x7E, 0x60, 0x60, 0x60, 0x7E, 0x60, 0x60, 0x60, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x7E, 0x66, 0x60, 0x6E, 0x66, 0x66, 0x66, 0x7E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x66, 0x7E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x66, 0x6C, 0x78, 0x70, 0x78, 0x6C, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x62, 0x62, 0x76, 0x76, 0x7E, 0x6A, 0x6A, 0x6A, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x72, 0x72, 0x7A, 0x6A, 0x6A, 0x6E, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x7E, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x7E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x7E, 0x66, 0x66, 0x66, 0x7E, 0x60, 0x60, 0x60, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x7E, 0x66, 0x66, 0x66, 0x66, 0x66, 0x74, 0x78, 0x0C, 0x06, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x7E, 0x66, 0x66, 0x6C, 0x78, 0x6C, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x7E, 0x66, 0x60, 0x7E, 0x06, 0x06, 0x66, 0x7E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x7E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x62, 0x6A, 0x6A, 0x6A, 0x6A, 0x7E, 0x34, 0x34, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x66, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x7E, 0x66, 0x0C, 0x18, 0x30, 0x60, 0x66, 0x7E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x3C, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3C, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x60, 0x60, 0x30, 0x30, 0x18, 0x18, 0x0C, 0x0C, 0x06, 0x06, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x3C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x3C, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x18, 0x3C, 0x66, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x06, 0x7E, 0x66, 0x66, 0x7E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x60, 0x60, 0x7E, 0x66, 0x66, 0x66, 0x66, 0x7E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x66, 0x60, 0x60, 0x66, 0x7E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x06, 0x06, 0x7E, 0x66, 0x66, 0x66, 0x66, 0x7E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x66, 0x7E, 0x60, 0x66, 0x7E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x3E, 0x30, 0x7E, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x66, 0x66, 0x66, 0x66, 0x7E, 0x06, 0x3E, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x60, 0x60, 0x7E, 0x66, 0x66, 0x66, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x78, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x1E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x66, 0x7E, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x60, 0x60, 0x66, 0x6C, 0x78, 0x6C, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x7E, 0x6A, 0x6A, 0x6A, 0x62, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x66, 0x66, 0x66, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x66, 0x66, 0x66, 0x66, 0x7E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x66, 0x66, 0x66, 0x66, 0x7E, 0x60, 0x60, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x66, 0x66, 0x66, 0x66, 0x7E, 0x06, 0x06, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x66, 0x60, 0x60, 0x60, 0x60, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x66, 0x70, 0x0E, 0x66, 0x7E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x7E, 0x30, 0x30, 0x30, 0x36, 0x3E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x7E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x62, 0x6A, 0x6A, 0x6A, 0x7E, 0x76, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x7E, 0x06, 0x3E, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x06, 0x0C, 0x30, 0x60, 0x7E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x0E, 0x18, 0x18, 0x18, 0x70, 0x70, 0x18, 0x18, 0x18, 0x0E, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x70, 0x18, 0x18, 0x18, 0x0E, 0x0E, 0x18, 0x18, 0x18, 0x70, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x7E, 0x4C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x32, 0x5A, 0x4C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

inline uint8_t colorToRRRGGGBB(uint32_t rgb) {
    uint8_t r = rgb >> 16;
    uint8_t g = rgb >> 8;
    uint8_t b = rgb;
    return (r & 0xE0) | ((g & 0xE0) >> 3) | ((b & 0xC0) >> 6);
}

bool video_set_mode(uint8_t mode) {
    if (video_mode == mode)
        return true;

    uint8_t* ret = vga_set_mode(mode);
    if (ret == 0) return false;
    vidmem = ret;
    video_mode = mode;
    switch (mode) {
        case 0:
            width = 80;
            height = 50;
            csr_x = saved_csr_x;
            csr_y = saved_csr_y;
            break;
        case 1:
            width = 80;
            height = 25;
            csr_x = saved_csr_x;
            csr_y = saved_csr_y;
            break;
        case 2:
            width = 320;
            height = 200;
            saved_csr_x = csr_x;
            saved_csr_y = csr_y;
            break;
    }
    return true;
}

uint8_t video_get_mode() {
    return video_mode;
}

void set_cursor(uint16_t x, uint16_t y) {
    csr_x = x;
    csr_y = y;
    if (video_mode <= 1) {
        vga_update_cursor(width * y + x);
    }
}

void move_cursor_right() {
    if ((csr_x + 1) >= width) {
        if ((csr_y + 1) < height) {
            set_cursor(0, csr_y + 1);
        }
    } else {
        set_cursor(csr_x + 1, csr_y);
    }
}

void move_cursor_left() {
    if (csr_x) {
        set_cursor(csr_x - 1, csr_y);
    } else if (csr_y) {
        set_cursor(width - 1, csr_y - 1);
    }
}

void move_cursor_home() {
    set_cursor(0, csr_y);
}

void move_cursor_end() {
    set_cursor(width - 1, csr_y);
}

void clear_screen() {
    if (video_mode <= 1) {
        memsetw((uint16_t*)vidmem, 0x20 | (txtcolor << 8), width * height);
    } else if (video_mode == 2) {
        memset(vidmem, 0, width * height);
    }
    set_cursor(0, 0);
}

static uint8_t transferFromAsciiToCodepage437(uint8_t ascii) {
    uint8_t c;

    if (ascii == 0xE4)
        c = 0x84;  // ä
    else if (ascii == 0xF6)
        c = 0x94;  // ö
    else if (ascii == 0xFC)
        c = 0x81;  // ü
    else if (ascii == 0xDF)
        c = 0xE1;  // ß
    else if (ascii == 0xA7)
        c = 0x15;  // §
    else if (ascii == 0xB0)
        c = 0xF8;  // °
    else if (ascii == 0xC4)
        c = 0x8E;  // Ä
    else if (ascii == 0xD6)
        c = 0x99;  // Ö
    else if (ascii == 0xDC)
        c = 0x9A;  // Ü
    else if (ascii == 0xB2)
        c = 0xFD;  // ²
    else if (ascii == 0xB3)
        c = 0x00;  // ³ <-- not available
    else if (ascii == 0x80)
        c = 0xEE;  // € <-- Greek epsilon used
    else if (ascii == 0xB5)
        c = 0xE6;  // µ
    else
        c = ascii;

    return c;
}

void putch(char c) {
    if (video_mode <= 1) {
        uint8_t uc = transferFromAsciiToCodepage437((uint8_t)c);  // no negative values

        uint16_t* pos;

        if (uc == '\b') {  // backspace: move the cursor one space backwards and delete
            if (csr_x) {
                --csr_x;
                pos = (uint16_t*)vidmem + (csr_y * width + csr_x);
                *pos = ' ' | ((txtcolor & 0xFF) << 8);
            } else if (!csr_x && csr_y > 0) {
                csr_x = width - 1;
                --csr_y;
                pos = (uint16_t*)vidmem + (csr_y * width + csr_x);
                *pos = ' ' | ((txtcolor & 0xFF) << 8);
            }
        } else if (uc == '\t')  // tab: increment csr_x (divisible by 8)
            csr_x = (csr_x + 8) & ~(8 - 1);
        else if (uc == '\r')  // cr: cursor back to the margin
            csr_x = 0;
        else if (uc == '\n') {  // newline: like 'cr': cursor to the margin and increment csr_y
            csr_x = 0;
            ++csr_y;
        } else if (uc != 0) {
            pos = (uint16_t*)vidmem + (csr_y * width + csr_x);
            *pos = uc | ((txtcolor & 0xFF) << 8);  // character AND attributes: color
            ++csr_x;
        }

        if (csr_x >= width) {  // cursor reaches edge of the screen's width, a new line is inserted
            csr_x = 0;
            ++csr_y;
        }

        // scroll if needed, and finally move the cursor
        scroll();
        vga_update_cursor(csr_y * width + csr_x);
    } else if (video_mode == 2) {
        if (c >= 33 && c <= 127) {
            uint8_t colorindex = colorToRRRGGGBB(color);
            for (uint8_t i = 0; i < 16; ++i) {
                for (uint8_t j = 0; j < 8; ++j) {
                    if (csr_x + j < 0 || width <= csr_x + j || csr_y + i < 0 || height <= csr_y + i) {
                        continue;
                    }
                    if (vga_font_8x16[(c - 33) * 16 + i] & (0x80 >> j)) {
                        vidmem[width * (csr_y + i) + csr_x + j] = colorindex;
                    }
                }
            }
        }
        csr_x += 8;
        if (csr_x >= width)  // cursor reaches edge of the screen's width, a new line is inserted
        {
            csr_x = 0;
            csr_y += 16;
        }
    }
}

void settextcolor(uint8_t forecolor, uint8_t backcolor) {
    // Top 4 bytes: background, bottom 4 bytes: foreground color
    txtcolor = (backcolor << 4) | (forecolor & 0x0F);
}

void setgraphcolor(uint32_t rgb) {
    color = rgb;
}

void scroll() {
    if (video_mode <= 1) {
        if (csr_y >= height) {
            uint32_t temp = csr_y - height + 1;
            memcpy(vidmem, vidmem + temp * width * 2, (height - temp) * width * 2);
            memsetw((uint16_t*)vidmem + (height - temp) * width, ' ' | ((txtcolor & 0xFF) << 8), width);
            csr_y = height - 1;
        }
    }
}

void puts(const char* text) {
    for (; *text; putch(*text), ++text)
        ;
}

typedef enum {
    PRINTF_FLAG_PREFIX      = 0x00000001,
    PRINTF_FLAG_SIGNED      = 0x00000002,
    PRINTF_FLAG_ZEROPADDED  = 0x00000004,
    PRINTF_FLAG_LEFTALIGNED = 0x00000010,
    PRINTF_FLAG_SHOWPLUS    = 0x00000020,
    PRINTF_FLAG_SPACESIGN   = 0x00000040,
    PRINTF_FLAG_BIGCHARS    = 0x00000080,
    PRINTF_FLAG_NEGATIVE    = 0x00000100
} printf_flag_t;

typedef enum {
    PRITNF_QUALIFIER_BYTE = 0,
    PRITNF_QUALIFIER_SHORT,
    PRITNF_QUALIFIER_INT,
    PRITNF_QUALIFIER_LONG,
    PRITNF_QUALIFIER_LONG_LONG,
    PRITNF_QUALIFIER_POINTER,
    PRITNF_QUALIFIER_SIZE,
    PRITNF_QUALIFIER_MAX
} printf_qualifier_t;

#define PRINTF_GET_INT_ARGUMENT(type, ap, flags) ({  \
    unsigned type res;                               \
                                                     \
    if ((flags)&PRINTF_FLAG_SIGNED) {                \
        signed type arg = va_arg((ap), signed type); \
                                                     \
        if (arg < 0) {                               \
            res = -arg;                              \
            (flags) |= PRINTF_FLAG_NEGATIVE;         \
        } else {                                     \
            res = arg;                               \
        }                                            \
    } else {                                         \
        res = va_arg((ap), unsigned type);           \
    }                                                \
                                                     \
    res;                                             \
})

void printf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    size_t cur;             // Index of the currently processed character from fmt
    size_t nxt = 0;         // Index of the next character from fmt
    size_t firstNoFmt = 0;  // Index to the first not printed nonformating character

    for (;;) {
        cur = nxt;
        char c = fmt[nxt++];

        if (c == 0)
            break;

        // Control character
        if (c == '%') {
            // Print common characters if any processed
            if (cur > firstNoFmt) {
                for (uint32_t i = firstNoFmt; i < cur; ++i) {
                    if (fmt[i] == 0)
                        break;

                    putch(fmt[i]);
                }
            }

            firstNoFmt = cur;

            // Parse modifiers
            uint32_t flags = 0;
            bool loopEnd = false;

            do {
                cur = nxt;
                c = fmt[nxt++];

                switch (c) {
                    case '#':
                        flags |= PRINTF_FLAG_PREFIX;
                        break;
                    case '-':
                        flags |= PRINTF_FLAG_LEFTALIGNED;
                        break;
                    case '+':
                        flags |= PRINTF_FLAG_SHOWPLUS;
                        break;
                    case ' ':
                        flags |= PRINTF_FLAG_SPACESIGN;
                        break;
                    case '0':
                        flags |= PRINTF_FLAG_ZEROPADDED;
                        break;
                    default:
                        loopEnd = true;
                }
            } while (!loopEnd);

            // Width & '*' operator
            int32_t width = 0;

            if (c >= '0' && c <= '9') {
                for (;;) {
                    width *= 10;
                    width += c - '0';

                    cur = nxt;
                    c = fmt[nxt++];

                    if (c == 0)
                        break;

                    if (!(c >= '0' && c <= '9'))
                        break;
                }
            } else if (c == '*') {
                // Get width value from argument list
                cur = nxt;
                c = fmt[nxt++];
                width = va_arg(ap, int32_t);

                if (width < 0) {
                    // Negative width sets '-' flag
                    width *= -1;
                    flags |= PRINTF_FLAG_LEFTALIGNED;
                }
            }

            // Precision and '*' operator
            int32_t precision = 0;

            if (c == '.') {
                cur = nxt;
                c = fmt[nxt++];

                if (c >= '0' && c <= '9') {
                    for (;;) {
                        precision *= 10;
                        precision += c - '0';

                        cur = nxt;
                        c = fmt[nxt++];

                        if (c == 0)
                            break;

                        if (!(c >= '0' && c <= '9'))
                            break;
                    }
                } else if (c == '*') {
                    // Get precision value from the argument list
                    cur = nxt;
                    c = fmt[nxt++];
                    precision = va_arg(ap, int32_t);

                    if (precision < 0)
                        // Ignore negative precision
                        precision = 0;
                }
            } else {
                precision = -1;
            }

            printf_qualifier_t qualifier;

            switch (c) {
                case 't':
                    // ptrdiff_t
                    qualifier = (sizeof(void*) == sizeof(int32_t)) ? PRITNF_QUALIFIER_INT : PRITNF_QUALIFIER_LONG_LONG;
                    cur = nxt;
                    c = fmt[nxt++];
                    break;
                case 'h':
                    // Char or short
                    qualifier = PRITNF_QUALIFIER_SHORT;
                    cur = nxt;
                    c = fmt[nxt++];

                    if (c == 'h') {
                        cur = nxt;
                        c = fmt[nxt++];
                        qualifier = PRITNF_QUALIFIER_BYTE;
                    }
                    break;
                case 'l':
                    // Long or long long
                    qualifier = PRITNF_QUALIFIER_LONG;
                    cur = nxt;
                    c = fmt[nxt++];

                    if (c == 'l') {
                        cur = nxt;
                        c = fmt[nxt++];
                        qualifier = PRITNF_QUALIFIER_LONG_LONG;
                    }
                    break;
                case 'z':
                    qualifier = PRITNF_QUALIFIER_SIZE;
                    cur = nxt;
                    c = fmt[nxt++];
                    break;
                case 'j':
                    qualifier = PRITNF_QUALIFIER_MAX;
                    cur = nxt;
                    c = fmt[nxt++];
                    break;
                default:
                    // Default type
                    qualifier = PRITNF_QUALIFIER_INT;
            }

            uint8_t base = 10;

            switch (c) {
                case 's': {
                    char* str = va_arg(ap, char*);

                    if (str == NULL) {
                        putch('(');
                        putch('N');
                        putch('U');
                        putch('L');
                        putch('L');
                        putch(')');
                    } else {
                        // Print leading spaces.
                        size_t strw = strlen(str);

                        if (precision == 0 || precision > strw)
                            precision = strw;

                        // Left padding
                        width -= precision;

                        if (!(flags & PRINTF_FLAG_LEFTALIGNED)) {
                            while (width-- > 0)
                                putch(' ');
                        }

                        // Part of @a str fitting into the alloted space.
                        for (uint32_t i = 0; i < precision; ++i) {
                            if (str[i] == 0)
                                break;

                            putch(str[i]);
                        }

                        // Right padding
                        while (width-- > 0)
                            putch(' ');
                    }

                    firstNoFmt = nxt;
                    continue;
                }
                case 'c':
                    if (!(flags & PRINTF_FLAG_LEFTALIGNED)) {
                        while (--width > 0)
                            // One space is consumed by the character itself, hence the predecrement.
                            putch(' ');
                    }

                    putch(va_arg(ap, unsigned int));

                    while (--width > 0)
                        // One space is consumed by the character itself, hence the predecrement.
                        putch(' ');

                    firstNoFmt = nxt;
                    continue;
                case 'P':
                    flags |= PRINTF_FLAG_BIGCHARS;
                    /* Fallthrough */
                case 'p':
                    flags |= PRINTF_FLAG_PREFIX;
                    flags |= PRINTF_FLAG_ZEROPADDED;
                    base = 16;
                    qualifier = PRITNF_QUALIFIER_POINTER;
                    break;
                case 'b':
                    base = 2;
                    break;
                case 'o':
                    base = 8;
                    break;
                case 'd':
                case 'i':
                    flags |= PRINTF_FLAG_SIGNED;
                    /* Fallthrough */
                case 'u':
                    break;
                case 'X':
                    flags |= PRINTF_FLAG_BIGCHARS;
                    /* Fallthrough */
                case 'x':
                    base = 16;
                    break;
                case 'F':
                    flags |= PRINTF_FLAG_BIGCHARS;
                    /* Fallthrough */
                case 'f': {
                    char str[32];
                    size_t retval = ftoa(va_arg(ap, double), str, (precision >= 0) ? precision : 6);

                    if ((flags & PRINTF_FLAG_BIGCHARS) && (str[0] == 'n' || str[0] == 'i'))
                        strupr(str);

                    width -= retval;

                    if (!(flags & PRINTF_FLAG_LEFTALIGNED)) {
                        char ch = (flags & PRINTF_FLAG_ZEROPADDED) ? '0' : ' ';

                        while (width-- > 0)
                            putch(ch);
                    }

                    for (uint32_t i = 0; i < retval; ++i)
                        putch(str[i]);

                    while (width-- > 0)
                        putch(' ');

                    firstNoFmt = nxt;
                    continue;
                }
                case '%':
                    firstNoFmt = cur;
                    continue;
                default:
                    continue;
            }

            // Print integers
            uint64_t number;

            switch (qualifier) {
                case PRITNF_QUALIFIER_BYTE:
                    number = PRINTF_GET_INT_ARGUMENT(int, ap, flags);
                    break;
                case PRITNF_QUALIFIER_SHORT:
                    number = PRINTF_GET_INT_ARGUMENT(int, ap, flags);
                    break;
                case PRITNF_QUALIFIER_INT:
                    number = PRINTF_GET_INT_ARGUMENT(int, ap, flags);
                    break;
                case PRITNF_QUALIFIER_LONG:
                    number = PRINTF_GET_INT_ARGUMENT(long, ap, flags);
                    break;
                case PRITNF_QUALIFIER_LONG_LONG:
                    number = PRINTF_GET_INT_ARGUMENT(long long, ap, flags);
                    break;
                case PRITNF_QUALIFIER_POINTER:
                    precision = sizeof(void*) << 1;
                    number = (uint32_t)(va_arg(ap, void*));
                    break;
                case PRITNF_QUALIFIER_SIZE:
                    number = va_arg(ap, size_t);
                    break;
                case PRITNF_QUALIFIER_MAX:
                    number = va_arg(ap, uintmax_t);
                    break;
                default:
                    // Unknown qualifier
                    return;
            }

            static const char* digits_small = "0123456789abcdef";
            static const char* digits_big = "0123456789ABCDEF";

            const char* digits = (flags & PRINTF_FLAG_BIGCHARS) ? digits_big : digits_small;
            char data[69];
            char* ptr = &data[68];

            if (precision < 0)
                precision = 0;

            // Size of number with all prefixes and signs
            int size = 0;

            // Put zero at end of string
            *ptr-- = 0;

            if (number == 0) {
                *ptr-- = '0';
                size++;
            } else {
                do {
                    *ptr-- = digits[number % base];
                    size++;
                } while (number /= base);
            }

            // Size of plain number
            int number_size = size;

            // Collect the sum of all prefixes/signs/etc. to calculate padding and leading zeroes.
            if (flags & PRINTF_FLAG_PREFIX) {
                switch (base) {
                    case 2:
                        // Binary formating is not standard, but usefull
                        size += 2;
                        break;
                    case 8:
                        size++;
                        break;
                    case 16:
                        size += 2;
                        break;
                }
            }

            char sgn = 0;

            if (flags & PRINTF_FLAG_SIGNED) {
                if (flags & PRINTF_FLAG_NEGATIVE) {
                    sgn = '-';
                    size++;
                } else if (flags & PRINTF_FLAG_SHOWPLUS) {
                    sgn = '+';
                    size++;
                } else if (flags & PRINTF_FLAG_SPACESIGN) {
                    sgn = ' ';
                    size++;
                }
            }

            if (flags & PRINTF_FLAG_LEFTALIGNED)
                flags &= ~PRINTF_FLAG_ZEROPADDED;

            /*
            * If the number is left-aligned or precision is specified then
            * padding with zeros is ignored.
            */
            if (flags & PRINTF_FLAG_ZEROPADDED) {
                if ((precision == 0) && (width > size))
                    precision = width - size + number_size;
            }

            // Print leading spaces
            if (number_size > precision)
                // Print the whole number, not only a part
                precision = number_size;

            width -= precision + size - number_size;

            if (!(flags & PRINTF_FLAG_LEFTALIGNED)) {
                while (width-- > 0)
                    putch(' ');
            }

            // Print sign
            if (sgn)
                putch(sgn);

            // Print prefix
            if (flags & PRINTF_FLAG_PREFIX) {
                switch (base) {
                    case 2:
                        // Binary formating is not standard, but usefull
                        putch('0');
                        putch((flags & PRINTF_FLAG_BIGCHARS) ? 'B' : 'b');
                        break;
                    case 8:
                        putch('0');
                        break;
                    case 16:
                        putch('0');
                        putch((flags & PRINTF_FLAG_BIGCHARS) ? 'X' : 'x');
                        break;
                }
            }

            // Print leading zeroes
            precision -= number_size;

            while (precision-- > 0)
                putch('0');

            // Print the number itself
            ptr++;

            while (*ptr != 0)
                putch(*(ptr++));

            // Print trailing spaces
            while (width-- > 0)
                putch(' ');

            firstNoFmt = nxt;
        }
    }

    if (cur > firstNoFmt) {
        for (uint32_t i = firstNoFmt; i < cur; ++i) {
            if (fmt[i] == 0)
                break;

            putch(fmt[i]);
        }
    }
}

void draw_rectangle(int16_t x, int16_t y, int16_t w, int16_t h) {
    if (video_mode == 2) {
        if (w <= 0 || h <= 0) return;
        int16_t X;
        int16_t Y;
        int16_t Xmax = MIN(x + w, width);
        int16_t Ymax = MIN(y + h, height);
        uint8_t colorindex = colorToRRRGGGBB(color);
        uint32_t a = (color >> 24) & 0xFF;
        if (a == 0xFF) {
            for (Y = MAX(y, 0); Y < Ymax; ++Y) {
                for (X = MAX(x, 0); X < Xmax; ++X) {
                    vidmem[width * Y + X] = colorindex;
                }
            }
        } else {
            uint32_t r = (color >> 16) & 0xFF;
            uint32_t g = (color >> 8) & 0xFF;
            uint32_t b = color & 0xFF;
            for (Y = MAX(y, 0); Y < Ymax; ++Y) {
                for (X = MAX(x, 0); X < Xmax; ++X) {
                    uint8_t prev_col = vidmem[width * Y + X];
                    uint32_t r_prev = prev_col & 0xE0;
                    uint32_t g_prev = (prev_col << 3) & 0xE0;
                    uint32_t b_prev = prev_col << 6;
                    uint32_t r_new = (a * r + (255 - a) * r_prev) / 255;
                    uint32_t g_new = (a * g + (255 - a) * g_prev) / 255;
                    uint32_t b_new = (a * b + (255 - a) * b_prev) / 255;
                    vidmem[width * Y + X] = (r_new & 0xE0) | ((g_new & 0xE0) >> 3) | ((b_new & 0xC0) >> 6);
                }
            }
        }
    }
}

void put_pixel(int16_t x, int16_t y) {
    if (video_mode == 2) {
        if (x < 0 || width <= x || y < 0 || height <= y) {
            return;
        }
        vidmem[width * y + x] = colorToRRRGGGBB(color);
    }
}

void draw_picture_part(const uint8_t* picture, int16_t x, int16_t y, int16_t xoffset, int16_t yoffset, uint16_t w, uint16_t h) {
    if (video_mode == 2) {
        uint16_t W = (picture[0] << 8) + picture[1];
        uint16_t H = (picture[2] << 8) + picture[3];
        int16_t maxH = MIN(MIN(y + H, y + yoffset + h), height);
        int16_t maxW = MIN(MIN(x + W, x + xoffset + w), width);
        xoffset = MAX(x + MAX(xoffset, 0), 0);
        yoffset = MAX(y + MAX(yoffset, 0), 0);
        for (int16_t j = yoffset; j < maxH; ++j) {
            for (int16_t i = xoffset; i < maxW; ++i) {
                vidmem[width * j + i] = picture[4 + (j - y) * W + (i - x)];
            }
        }
    }
}

void draw_picture(const uint8_t* picture, int16_t x, int16_t y) {
    draw_picture_part(picture, x, y, 0, 0, width, height);
}

void draw_picture_part32(const uint32_t* picture, int16_t x, int16_t y, int16_t xoffset, int16_t yoffset, uint16_t w, uint16_t h) {
    if (video_mode == 2) {
        uint16_t W = picture[0];
        uint16_t H = picture[1];
        int16_t maxH = MIN(MIN(y + H, y + yoffset + h), height);
        int16_t maxW = MIN(MIN(x + W, x + xoffset + w), width);
        xoffset = MAX(x + MAX(xoffset, 0), 0);
        yoffset = MAX(y + MAX(yoffset, 0), 0);
        for (int16_t j = yoffset; j < maxH; ++j) {
            for (int16_t i = xoffset; i < maxW; ++i) {
                if (picture[2 + (j - y) * W + (i - x)] & 0xFF000000) vidmem[width * j + i] = colorToRRRGGGBB(picture[2 + (j - y) * W + (i - x)]);
            }
        }
    }
}

void draw_picture32(const uint32_t* picture, int16_t x, int16_t y) {
    draw_picture_part32(picture, x, y, 0, 0, width, height);
}

void draw_text_part(int16_t x, int16_t y, const char* text, int16_t xoffset, int16_t yoffset, int16_t w, int16_t h) {
    if (video_mode == 2) {
        xoffset = CLAMP(xoffset, 0, width - 1);
        yoffset = CLAMP(yoffset, 0, height - 1);
        w = MIN(xoffset + w, width);
        h = MIN(yoffset + h, height);
        uint8_t colorindex = colorToRRRGGGBB(color);
        while (*text != 0) {
            if (*text >= 33 && *text <= 127) {
                for (uint8_t i = 0; i < 16; ++i) {
                    for (uint8_t j = 0; j < 8; ++j) {
                        if (((x + j) >= xoffset) && ((x + j) < w) && ((y + i) >= yoffset) && ((y + i) < h)) {
                            if (vga_font_8x16[(*text - 33) * 16 + i] & (0x80 >> j)) {
                                vidmem[width * (y + i) + x + j] = colorindex;
                            }
                        }
                    }
                }
            }
            x += 8;
            ++text;
        }
    }
}
