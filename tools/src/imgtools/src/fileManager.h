#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <stdint.h>

struct Partition_t;
typedef struct file_t {
    char name[35];
    uint8_t data[32];
    uint32_t size;
    uint8_t attribute;
    struct Partition_t* partition; // Partition the file belongs to
} file_t;

#endif
