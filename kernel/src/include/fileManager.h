#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "os.h"
#include "partitionManager.h"

struct Partition_t;
typedef struct file_t {
    char name[35];
    uint8_t data[32];
    uint32_t size;
    uint8_t attribute;
    struct Partition_t* partition;  // Partition the file belongs to
} file_t;

uint8_t file_create(file_t* file_inst, char* filepath);
uint8_t file_find(file_t* file_inst, char* filepath);
void file_readContents(file_t* file_inst, uint8_t* buffer, uint32_t start, uint32_t len);
uint8_t file_isDirectory(char* filepath);
uint8_t file_createDirectory(char* filepath);
uint8_t file_findByIndex(file_t* file_inst, char* dirpath, uint32_t index);
uint8_t file_execute(char* filepath);
uint8_t file_delete(char* filepath);
void fileManage_getBootPath(char* filepath);

#endif
