#include "storage_devManager.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

FILE* ft = NULL;

void storage_devManager_init(const char* filename) {
    ft = fopen(filename, "rb+");
    if (ft == NULL)
    {
        fprintf(stderr, "cannot open target file %s\n", filename);
        exit(1);
    }
}

void storage_bye() {
    fclose(ft); //!!! MEMORY LEAK !!!
}

uint32_t storage_size() {
    fseek(ft, 0L, SEEK_END);
    return ftell(ft);
}

void storage_readSector(storage_dev_t* dev, uint32_t sector, uint8_t* data, size_t count) {
    fseek(ft, sector * 512, SEEK_SET);
    fread(data, count, 1, ft);
}

void storage_writeSector(storage_dev_t* dev, uint32_t sector, uint8_t* data, size_t count) {
    fseek(ft, sector * 512, SEEK_SET);
    fwrite(data, 1, count, ft);
}
