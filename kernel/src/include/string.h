#ifndef STRING_H
#define STRING_H

#include "stdbool.h"
#include "stdint.h"

extern size_t strlen(const char* str);
extern int32_t strcmp(const char* s1, const char* s2);
extern char* strcpy(char* dest, const char* src);
extern char* strncpy(char* dest, const char* src, size_t n);
extern char* strcat(char* dest, const char* src);
extern char* strncat(char* dest, const char* src, size_t n);
extern char* strtok(char* str, const char* delim);
extern char* strlwr(char* str);
extern char* strupr(char* str);
extern char* strtrim(char* str);
extern char* strtrimstart(char* str);
extern char* strtrimend(char* str);
extern bool strstarts(const char* str, const char* prefix);
extern char* strrchr(const char* str, char c);
extern char* strstr(const char* string, const char* substring);
extern char* stringReplace(char* search, char* replace, char* string);
extern int32_t strncmp(const char* s1, const char* s2, size_t n);
extern char* strrev(char* str);

#endif
