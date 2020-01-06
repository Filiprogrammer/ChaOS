#ifndef ATA_H
#define ATA_H

#include "os.h"

#define ATADATAPORT 0
#define ATAERRORPORT 1
#define ATASECTORCOUNTPORT 2
#define ATALBALOWPORT 3
#define ATALBAMIDPORT 4
#define ATALBAHIPORT 5
#define ATADEVICEPORT 6
#define ATACOMMANDPORT 7
#define ATACONTROLPORT 0x206

typedef struct ATA {
    bool master;
    uint32_t bytesPerSector;
    uint16_t portBase;
} ATA;

bool ATA_identify(ATA* ata);
bool ATA_read(ATA* ata, uint32_t sector, uint8_t* data, size_t count);
bool ATA_write(ATA* ata, uint32_t sector, uint8_t* data, size_t count);
ATA* ATA_create(uint16_t portBase, bool master);
void ATA_destroy(ATA* ata);

#endif
