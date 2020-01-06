#ifndef USERLIB_H
#define USERLIB_H

#include "types.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define SIGN(X) ((0 < (X)) - ((X) < 0))
#define ABS(X) (((X) < 0) ? (-(X) : (X)))

typedef struct file_t {
    char name[35];
    uint8_t data[32];
    uint32_t size;
    uint8_t attribute;
    void* partition;  // Partition the file belongs to
} file_t;

#define FILE_ATTR_ARCHIVE 0x20
#define FILE_ATTR_VOLLABEL 0x08
#define FILE_ATTR_SUBDIR 0x10

#ifdef _cplusplus
extern "C" {
#endif

// syscalls
void settextcolor(uint8_t foreground, uint8_t background);
void putch(char val);
void puts(char* pString);
void sleepMilliSeconds(uint32_t ms);
bool isKeyDown(KEY_t key);
KEY_t getkey();
bool video_set_mode(uint8_t mode);
void put_pixel(int16_t x, int16_t y);
uint8_t file_createDirectory(char* filepath);
uint8_t file_create(file_t* file_inst, char* filepath);
void move_cursor_left();
void move_cursor_right();
uint8_t file_findByIndex(file_t* file_inst, char* dirpath, uint32_t index);
uint8_t file_isDirectory(char* filepath);
void desktop_enable();
uint8_t file_find(file_t* file_inst, char* filepath);
void file_readContents(file_t* file_inst, uint8_t* buffer, uint32_t start, uint32_t len);
void reboot();
void draw_picture(uint8_t* picture, int16_t x, int16_t y);
uint8_t file_delete(char* filepath);
char keyToASCII(KEY_t key);
uint32_t getMilliseconds();
void randomize();
void getBootPath(char* filepath);
void exit();
uint8_t file_execute(char* filepath);
bool pci_getDevice(uint32_t i, pciDev_t* pciDev);

// user functions
int32_t strcmp(const char* s1, const char* s2);
bool startsWith(char* str, char* prefix);
void substring(char* str, char* sub, int pos, int len);
size_t strlen(const char* str);
char* strcpy(char* dest, const char* src);
char* strcat(char* dest, const char* src);
char* strtok(char* str, const char* delim);
char* strtrim(char* str);
char* trimStart(char* str);
char* tolower(char* str);
char* strstr(char* string, char* substring);
char* file_squashPath(char* filepath);
int32_t power(int32_t base, int32_t n);
float powerf(float base, int32_t n);
int32_t fact(int32_t n);
float sin(float deg);
float cos(float deg);
void uitoa(uint32_t value, char* valuestring);
void itoa(int32_t value, char* valuestring);
void ftoa(float f, char* buffer);
float sqrt(float x);
uint32_t random();

#ifdef _cplusplus
}
#endif

#endif
