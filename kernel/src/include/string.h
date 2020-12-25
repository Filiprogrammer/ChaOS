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
extern char* tolower(char* str);
extern char* toupper(char* str);
extern char* strtrim(char* str);
extern char* strtrimstart(char* str);
extern char* strtrimend(char* str);
extern bool startsWith(char* str, char* prefix);
extern int32_t strrchr(char* str, char c);
extern char* strstr(char* string, char* substring);
extern char* stringReplace(char* search, char* replace, char* string);
extern int32_t strncmp(const char* s1, const char* s2, size_t n);
extern char* strrev(char* str);

#endif
