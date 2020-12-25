#include "partitionManager.h"

#include "fat.h"
#include "string.h"

extern uint32_t bootVolID;

static bool checkBootVol(uint8_t* volIDPtr) {
    if( ((uint8_t)bootVolID == volIDPtr[0])
    &&  ((uint8_t)(bootVolID >> 8) == volIDPtr[1])
    &&  ((uint8_t)(bootVolID >> 16) == volIDPtr[2])
    &&  ((uint8_t)(bootVolID >> 24) == volIDPtr[3])) {
        return true;
    }
    return false;
}

uint32_t PartManage_analyzeDev(storage_dev_t* dev) {
    uint32_t retBootPart = 0;
    uint8_t boot_sec[512];
    storage_readSector(dev, 0, boot_sec, 512);

    if ((boot_sec[510] == 0x55) && (boot_sec[511] == 0xAA)) {  // if there is a boot signature
        listHead_t* partList = 0;
        if (strncmp((char*)&boot_sec + 0x36, "FAT12   ", 8) == 0) {  // if it is a FAT12 formatted device
            partList = list_create();
            Partition_t* partition = malloc(sizeof(Partition_t), 0); // Not necessary to bother with freeing this
            partition->partEntry[0] = 0x80; // boot flag
            partition->partEntry[1] = 0; // startHead
            partition->partEntry[2] = 0x01; //startSectorCylinder
            partition->partEntry[3] = 0x00; //startSectorCylinder
            partition->partEntry[4] = 0x01; // type (FAT12)
            partition->partEntry[5] = 0; // TODO: endHead
            partition->partEntry[6] = 0; // TODO: endSectorCylinder
            partition->partEntry[7] = 0; // TODO: endSectorCylinder
            partition->partEntry[8] = 0; // lbaStart
            partition->partEntry[9] = 0; // lbaStart
            partition->partEntry[10] = 0; // lbaStart
            partition->partEntry[11] = 0; // lbaStart
            partition->partEntry[12] = boot_sec[0x20]; // lbaLength
            partition->partEntry[13] = boot_sec[0x21]; // lbaLength
            partition->partEntry[14] = boot_sec[0x22]; // lbaLength
            partition->partEntry[15] = boot_sec[0x23]; // lbaLength
            if ((partition->partEntry[12] == 0) && (partition->partEntry[13] == 0) && (partition->partEntry[14] == 0) && (partition->partEntry[15] == 0)) {
                partition->partEntry[12] = boot_sec[0x13];
                partition->partEntry[13] = boot_sec[0x14];
            }
            partition->inst = FAT_create(dev, 0, partition->partEntry[12] + (partition->partEntry[13] << 8) + (partition->partEntry[14] << 16) + (partition->partEntry[15] << 24), FAT12);
            partition->createFile = (void*)FAT_abstract_createFile;
            partition->findFile = (void*)FAT_abstract_findFile;
            partition->readFileContents = (void*)FAT_abstract_readFileContents;
            partition->isDirectory = (void*)FAT_abstract_isDirectory;
            partition->createDirectory = (void*)FAT_abstract_createDirectory;
            partition->findFileByIndex = (void*)FAT_readFileEntryByIndex;
            partition->deleteFile = (void*)FAT_deleteFile;
            list_append(partList, partition);
            if (checkBootVol(boot_sec + 0x27)) retBootPart = 1;
        } else if (strncmp((char*)&boot_sec + 0x36, "FAT16   ", 8) == 0) {  // if it is a FAT16 formatted device
            partList = list_create();
            Partition_t* partition = malloc(sizeof(Partition_t), 0); // Not necessary to bother with freeing this
            partition->partEntry[0] = 0x80; // boot flag
            partition->partEntry[1] = 0; // startHead
            partition->partEntry[2] = 0x01; //startSectorCylinder
            partition->partEntry[3] = 0x00; //startSectorCylinder
            partition->partEntry[4] = 0x04; // type (FAT16 <= 32 MiB)
            partition->partEntry[5] = 0; // TODO: endHead
            partition->partEntry[6] = 0; // TODO: endSectorCylinder
            partition->partEntry[7] = 0; // TODO: endSectorCylinder
            partition->partEntry[8] = 0; // lbaStart
            partition->partEntry[9] = 0; // lbaStart
            partition->partEntry[10] = 0; // lbaStart
            partition->partEntry[11] = 0; // lbaStart
            partition->partEntry[12] = boot_sec[0x20]; // lbaLength
            partition->partEntry[13] = boot_sec[0x21]; // lbaLength
            partition->partEntry[14] = boot_sec[0x22]; // lbaLength
            partition->partEntry[15] = boot_sec[0x23]; // lbaLength
            if ((partition->partEntry[12] == 0) && (partition->partEntry[13] == 0) && (partition->partEntry[14] == 0) && (partition->partEntry[15] == 0)) {
                partition->partEntry[12] = boot_sec[0x13];
                partition->partEntry[13] = boot_sec[0x14];
                partition->partEntry[4] = 0x06;  // type (FAT16 > 32 MiB)
            }
            partition->inst = FAT_create(dev, 0, partition->partEntry[12] + (partition->partEntry[13] << 8) + (partition->partEntry[14] << 16) + (partition->partEntry[15] << 24), FAT16);
            partition->createFile = (void*)FAT_abstract_createFile;
            partition->findFile = (void*)FAT_abstract_findFile;
            partition->readFileContents = (void*)FAT_abstract_readFileContents;
            partition->isDirectory = (void*)FAT_abstract_isDirectory;
            partition->createDirectory = (void*)FAT_abstract_createDirectory;
            partition->findFileByIndex = (void*)FAT_readFileEntryByIndex;
            partition->deleteFile = (void*)FAT_deleteFile;
            list_append(partList, partition);
            if (checkBootVol(boot_sec + 0x27)) retBootPart = 1;
        } else if (strncmp((char*)&boot_sec + 0x52, "FAT32   ", 8) == 0) {  // if it is a FAT32 formatted device
            partList = list_create();
            Partition_t* partition = malloc(sizeof(Partition_t), 0); // Not necessary to bother with freeing this
            partition->partEntry[0] = 0x80; // boot flag
            partition->partEntry[1] = 0; // startHead
            partition->partEntry[2] = 0x01; //startSectorCylinder
            partition->partEntry[3] = 0x00; //startSectorCylinder
            partition->partEntry[4] = 0x0B; // type (FAT32)
            partition->partEntry[5] = 0; // TODO: endHead
            partition->partEntry[6] = 0; // TODO: endSectorCylinder
            partition->partEntry[7] = 0; // TODO: endSectorCylinder
            partition->partEntry[8] = 0; // lbaStart
            partition->partEntry[9] = 0; // lbaStart
            partition->partEntry[10] = 0; // lbaStart
            partition->partEntry[11] = 0; // lbaStart
            partition->partEntry[12] = boot_sec[0x20]; // lbaLength
            partition->partEntry[13] = boot_sec[0x21]; // lbaLength
            partition->partEntry[14] = boot_sec[0x22]; // lbaLength
            partition->partEntry[15] = boot_sec[0x23]; // lbaLength
            partition->inst = FAT_create(dev, 0, partition->partEntry[12] + (partition->partEntry[13] << 8) + (partition->partEntry[14] << 16) + (partition->partEntry[15] << 24), FAT32);
            partition->createFile = (void*)FAT_abstract_createFile;
            partition->findFile = (void*)FAT_abstract_findFile;
            partition->readFileContents = (void*)FAT_abstract_readFileContents;
            partition->isDirectory = (void*)FAT_abstract_isDirectory;
            partition->createDirectory = (void*)FAT_abstract_createDirectory;
            partition->findFileByIndex = (void*)FAT_readFileEntryByIndex;
            partition->deleteFile = (void*)FAT_deleteFile;
            list_append(partList, partition);
            if (checkBootVol(boot_sec + 0x43)) retBootPart = 1;
        } else {  // if device has a Master Boot Record
            partList = list_create();
            for (uint8_t i = 0; i < 4; ++i) {
                if ((boot_sec[0x1BE + 16 * i + 12] == 0) && (boot_sec[0x1BE + 16 * i + 13] == 0) && (boot_sec[0x1BE + 16 * i + 14] == 0) && (boot_sec[0x1BE + 16 * i + 15] == 0)) continue;
                Partition_t* partition = malloc(sizeof(Partition_t), 0);  // Not necessary to bother with freeing this
                for (uint8_t j = 0; j < 16; ++j) {
                    partition->partEntry[j] = boot_sec[0x1BE + i * 16 + j];
                }
                switch (partition->partEntry[4]) {
                    case 0x0B:  //FAT32
                        partition->inst = FAT_create(dev,
                        partition->partEntry[8] + (partition->partEntry[9] << 8) + (partition->partEntry[10] << 16) + (partition->partEntry[11] << 24),
                        partition->partEntry[12] + (partition->partEntry[13] << 8) + (partition->partEntry[14] << 16) + (partition->partEntry[15] << 24),
                        FAT32);
                        partition->createFile = (void*)FAT_abstract_createFile;
                        partition->findFile = (void*)FAT_abstract_findFile;
                        partition->readFileContents = (void*)FAT_abstract_readFileContents;
                        partition->isDirectory = (void*)FAT_abstract_isDirectory;
                        partition->createDirectory = (void*)FAT_abstract_createDirectory;
                        partition->findFileByIndex = (void*)FAT_readFileEntryByIndex;
                        partition->deleteFile = (void*)FAT_deleteFile;
                        break;
                    case 0x01:  //FAT12
                        partition->inst = FAT_create(dev,
                        partition->partEntry[8] + (partition->partEntry[9] << 8) + (partition->partEntry[10] << 16) + (partition->partEntry[11] << 24),
                        partition->partEntry[12] + (partition->partEntry[13] << 8) + (partition->partEntry[14] << 16) + (partition->partEntry[15] << 24),
                        FAT12);
                        partition->createFile = (void*)FAT_abstract_createFile;
                        partition->findFile = (void*)FAT_abstract_findFile;
                        partition->readFileContents = (void*)FAT_abstract_readFileContents;
                        partition->isDirectory = (void*)FAT_abstract_isDirectory;
                        partition->createDirectory = (void*)FAT_abstract_createDirectory;
                        partition->findFileByIndex = (void*)FAT_readFileEntryByIndex;
                        partition->deleteFile = (void*)FAT_deleteFile;
                        break;
                    default:
                        partition->inst = 0;
                        partition->createFile = 0;
                        partition->findFile = 0;
                        partition->readFileContents = 0;
                        partition->isDirectory = 0;
                        partition->createDirectory = 0;
                        partition->findFileByIndex = 0;
                        partition->deleteFile = 0;
                        break;
                }
                list_append(partList, partition);
            }
        }

        if (dev->partitions != 0) {
            list_deleteAll(dev->partitions);
        }
        dev->partitions = partList;
    }
    return retBootPart;  // TODO: Even check for boot partition when there is a Master Boot Record
}

uint8_t PartManage_getPartType(uint8_t* partEntry) {
    return partEntry[4];
}

uint32_t PartManage_getPartStartSector(uint8_t* partEntry) {
    return partEntry[8] + (partEntry[9] << 8) + (partEntry[10] << 16) + (partEntry[11] << 24);
}

uint32_t PartManage_getPartLength(uint8_t* partEntry) {
    return partEntry[12] + (partEntry[13] << 8) + (partEntry[14] << 16) + (partEntry[15] << 24);
}
