#ifndef STORAGE_DEVMANAGER_H
#define STORAGE_DEVMANAGER_H

#include <stdint.h>
#include <stddef.h>

#define STORAGE_DEVMANAGER_TYPE_FLOPPY 0
#define STORAGE_DEVMANAGER_TYPE_HDD 1

typedef struct {
	uint8_t type;
	void* inst;
	// Parameters: inst, sector, data, count
	uint8_t(*readSector) (void*, uint32_t, uint8_t*, size_t);
	uint8_t(*writeSector)(void*, uint32_t, uint8_t*, size_t);
	void* partitions;
} storage_dev_t;

void storage_devManager_init(const char* filename);
void storage_readSector(storage_dev_t* dev, uint32_t sector, uint8_t* data, size_t count);
void storage_writeSector(storage_dev_t* dev, uint32_t sector, uint8_t* data, size_t count);

void storage_bye();
uint32_t storage_size();

#endif
