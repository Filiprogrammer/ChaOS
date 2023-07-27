#ifndef FAT_H
#define FAT_H

#include <stdint.h>
#include "storage_devManager.h"
#include "fileManager.h"

typedef struct FAT {
    storage_dev_t* storageDev;
    uint32_t lbaStart;
    uint32_t lbaLength;
    uint16_t reservedSectors;
    uint32_t sectorsPerFAT;
    uint8_t FATCopies;
    uint8_t sectorsPerCluster;
    uint32_t startClustOfRootDir;
    uint8_t fileSys;
    uint16_t maxRootDirEntries;
}FAT;

#define FAT12 0
#define FAT16 1
#define FAT32 2
#define FAT_ATTR_ARCHIVE 0x20
#define FAT_ATTR_VOLLABEL 0x08
#define FAT_ATTR_SUBDIR 0x10
#define FAT_MAX_PATH_FILES 16
#define FAT_MAX_FNAME_LEN 34

FAT* FAT_create(storage_dev_t* storageDev, uint32_t lbaStart, uint32_t lbaLength, uint8_t fileSys);
void FAT_destroy(FAT* inst);
uint8_t FAT_writeFileContents(FAT* inst, char* filepath, uint8_t* contents, uint32_t length);
char* FAT_readVolumeLabel(FAT* inst);
char* FAT_formatFilePath(char* filepath);
uint8_t* FAT_readFileEntry(FAT* inst, char* filepath);
uint32_t FAT_getFilesize(uint8_t* entry);
uint8_t FAT_readFileByte(FAT* inst, uint8_t* entry);
void FAT_readFileContents(FAT* inst, uint8_t* entry, uint8_t* buffer, uint32_t start, uint32_t len);
uint8_t FAT_format(FAT* inst, uint8_t fileSys, char* volumeLabel, uint16_t reservedSectors, uint8_t quick);
uint8_t* FAT_createFile(FAT* inst, char* filepath, uint8_t attribute);
void FAT_setVolumeLabel(FAT* inst, char* volumeLabel);
uint8_t FAT_createDirectory(FAT* inst, char* filepath);
void FAT_deleteFile(FAT* inst, char* filepath);

uint8_t FAT_readFileEntryByIndex(FAT* inst, file_t* file_inst, char* filepath, uint32_t index);

uint8_t FAT_abstract_createFile(FAT* inst, char* filepath, file_t* file_inst);
uint8_t FAT_abstract_findFile(FAT* inst, char* filepath, file_t* file_inst);
void FAT_abstract_readFileContents(FAT* inst, file_t* file_inst, uint8_t* buffer, uint32_t start, uint32_t len);
uint8_t FAT_abstract_isDirectory(FAT* inst, char* filepath);
uint8_t FAT_abstract_createDirectory(FAT* inst, char* filepath);

#endif
