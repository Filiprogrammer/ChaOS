#ifndef FLPYDSK_H
#define FLPYDSK_H

#include "os.h"

#define FLPY_DMA_BUFFER 0x1000
#define FLPY_DMA_BUFFER_LEN 0x4800

typedef struct {
    uint8_t drive;
} Flpydsk_t;

bool flpydsk_read_sectors(Flpydsk_t* inst, uint32_t sectorLBA, uint8_t* addr, size_t len);
bool flpydsk_write_sectors(Flpydsk_t* inst, uint32_t sectorLBA, uint8_t* addr, size_t len);
Flpydsk_t* flpydsk_create(uint8_t drive);
void flpydsk_destroy(Flpydsk_t* inst);

#endif
