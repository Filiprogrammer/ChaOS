#ifndef PARTITIONMANAGER_H
#define PARTITIONMANAGER_H

#include "fileManager.h"
#include "list.h"
#include "os.h"
#include "storage_devManager.h"

struct file_t;
typedef struct Partition_t {
    uint8_t partEntry[16];
    void* inst; // filesystem instance
    uint8_t (*createFile) (void*, char*, struct file_t*); // inst, char* filepath, file_t* file_inst
    uint8_t (*findFile)   (void*, char*, struct file_t*); // inst, char* filepath, file_t* file_inst
    void (*readFileContents) (void*, struct file_t*, uint8_t*, uint32_t, uint32_t); // fileSys inst, file_t* file_inst, uint8_t* buffer, uint32_t start, uint32_t len
    uint8_t (*isDirectory) (void*, char*); // fileSys inst, char* filepath
    uint8_t (*createDirectory) (void*, char*); // fileSys inst, char* filepath
    uint8_t (*findFileByIndex) (void*, struct file_t*, char*, uint32_t); // fileSys inst, file_t* file_inst, char* dirpath, uint32_t index
    void (*deleteFile) (void*, char*); // fileSys inst, char* filepath
} Partition_t;

uint32_t PartManage_analyzeDev(storage_dev_t* dev); // Returns a list of partitions
uint8_t PartManage_getPartType(uint8_t* partEntry);
uint32_t PartManage_getPartStartSector(uint8_t* partEntry);
uint32_t PartManage_getPartLength(uint8_t* partEntry);

#endif
