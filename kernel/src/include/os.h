#ifndef OS_H
#define OS_H

#include "stdbool.h"
#include "stddef.h"
#include "stdint.h"

#define PAGESIZE 4096

#define ASSERT(b) ((b) ? (void)0 : panic_assert(__FILE__, __LINE__, #b))
extern void panic_assert(char* file, uint32_t line, char* desc);

// This defines the operatings system common data area

typedef struct oda {
    // Hardware Data
    uint32_t COM1, COM2, COM3, COM4;  // address
    uint32_t LPT1, LPT2, LPT3, LPT4;  // address
    uint32_t Memory_Size;             // Memory size in Bytes

    //tasking
    uint8_t ts_flag;  // 0: taskswitch off  1: taskswitch on

    uint32_t bootDev;
    uint32_t bootPart;

    uint64_t cpu_frequency;
} oda_t;

// operatings system common data area
extern oda_t ODA;

// This defines what the stack looks like after an ISR was running
typedef struct regs {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} registers_t;

// video.c
extern bool video_set_mode(uint8_t mode);
extern uint8_t video_get_mode();
extern void set_cursor(uint16_t x, uint16_t y);
extern void move_cursor_right();
extern void move_cursor_left();
extern void move_cursor_home();
extern void move_cursor_end();
extern void clear_screen();
extern void settextcolor(uint8_t forecolor, uint8_t backcolor);
extern void setgraphcolor(uint32_t rgb);
extern void putch(char c);
extern void puts(char* text);
extern void scroll();
extern void printf(char* args, ...);
extern void draw_rectangle(int16_t x, int16_t y, int16_t w, int16_t h);
extern void put_pixel(int16_t x, int16_t y);
extern void draw_picture_part(const uint8_t* picture, int16_t x, int16_t y, int16_t xoffset, int16_t yoffset, uint16_t w, uint16_t h);
extern void draw_picture(const uint8_t* picture, int16_t x, int16_t y);
extern void draw_picture_part32(const uint32_t* picture, int16_t x, int16_t y, int16_t xoffset, int16_t yoffset, uint16_t w, uint16_t h);
extern void draw_picture32(const uint32_t* picture, int16_t x, int16_t y);
extern void draw_text_part(int16_t x, int16_t y, const char* text, int16_t xoffset, int16_t yoffset, int16_t w, int16_t h);

// timer.c
extern void timer_handler(registers_t* r);
extern uint32_t timer_getSeconds();
extern uint32_t timer_getMilliseconds();
void sleepSeconds(uint32_t seconds);
void sleepMilliSeconds(uint32_t ms);
void sleepMicroSeconds(uint32_t microsec);
uint32_t cpuCyclesToMicroSeconds(uint64_t cycles);
extern void systemTimer_setFrequency(uint32_t freq);
extern void timer_install();
extern void timer_uninstall();
extern uint64_t rdtsc();

// mouse.c
extern void mouse_install();
extern void mouse_setup(int16_t max_x, int16_t max_y, uint8_t slowness);
extern void mouse_set_bounds(int16_t max_x, int16_t max_y);
extern void mouse_switch(bool enable);
extern void mouse_add_move_listener(void (*move_event)(int16_t, int16_t));
extern void mouse_add_click_listener(void (*click_event)(uint8_t, bool));
extern void mouse_remove_move_listener(void (*move_event)(int16_t, int16_t));
extern void mouse_remove_click_listener(void (*click_event)(uint8_t, bool));

// util.c
extern void outportb(uint16_t port, uint8_t val);
extern uint8_t inportb(uint16_t port);
extern void outportw(uint16_t port, uint16_t val);
extern uint16_t inportw(uint16_t port);
extern void outportl(uint16_t port, uint32_t val);
extern uint32_t inportl(uint16_t port);
extern uint32_t fetchESP();
extern uint32_t fetchEBP();
extern uint32_t fetchSS();
extern uint32_t fetchCS();
extern uint32_t fetchDS();
extern uint32_t bitScanReverse(uint32_t val);
extern void memshow(void* start, size_t count);
extern void* memset(void* dest, int8_t val, size_t count);
extern uint16_t* memsetw(uint16_t* dest, uint16_t val, size_t count);
extern void* memcpy(void* dest, const void* src, size_t count);
extern uint8_t BSDChecksum(char* str);
extern void randomSetSeed(uint32_t val);
extern uint32_t random();
extern void reboot();
extern void cli();
extern void sti();
extern void nop();
extern void hlt();
extern void uitoa(uint32_t value, char* valuestring);
extern void itoa(int32_t value, char* valuestring);
extern void i2hex(uint32_t val, char* dest, int32_t len);
extern void ftoa(float value, int32_t decimal, char* valuestring);

// gtd.c itd.c irq.c isrs.c
extern void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
extern void gdt_install();
extern void idt_install();

extern void timer_install();
extern void keyboard_install();

extern registers_t* irq_handler(registers_t* esp);
extern void irq_install_handler(int32_t irq, void (*handler)(registers_t* r));
extern void irq_uninstall_handler(int32_t irq);

// heap.c
extern void heap_install();
extern void* malloc(uint32_t size, uint32_t alignment);
extern void free(void* mem);

//desktop.c
extern void desktop_enable();

#endif
