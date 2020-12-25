#include "ata.h"

#include "task.h"

/**
 * @brief Check wether the ATA storage device exists and if it does, try to identify it.
 * 
 * @param ata pointer to an ATA structure.
 * @return true storage device detected.
 * @return false no storge device detected.
 */
bool ATA_identify(ATA* ata) {
    outportb(ata->portBase + ATADEVICEPORT, ata->master ? 0xA0 : 0xB0);
    outportb(ata->portBase + ATACONTROLPORT, 0);

    outportb(ata->portBase + ATADEVICEPORT, 0xA0);
    uint8_t status = inportb(ata->portBase + ATACOMMANDPORT);
    if (status == 0xFF) return false;  //no device

    outportb(ata->portBase + ATADEVICEPORT, ata->master ? 0xA0 : 0xB0);
    outportb(ata->portBase + ATASECTORCOUNTPORT, 0);
    outportb(ata->portBase + ATALBALOWPORT, 0);
    outportb(ata->portBase + ATALBAMIDPORT, 0);
    outportb(ata->portBase + ATALBAHIPORT, 0);
    outportb(ata->portBase + ATACOMMANDPORT, 0xEC);

    status = inportb(ata->portBase + ATACOMMANDPORT);
    if (status == 0x00) return false;  //no device

    while (((status & 0x80) == 0x80) && ((status & 0x01) != 0x01))
        status = inportb(ata->portBase + ATACOMMANDPORT);

    if (status & 0x01) {
        //puts("ATA: ERROR\n");
        return false;  //ERROR
    }

    for (uint16_t i = 0; i < 256; ++i) {
        uint16_t data = inportw(ata->portBase + ATADATAPORT);
        if (i == 106) {
            if (data & 0x1000)
                ata->bytesPerSector = 0;  //Device Locigal Sector longer than 512 Bytes
            else
                ata->bytesPerSector = 512;
        } else if (i == 117) {
            if (ata->bytesPerSector == 0) ata->bytesPerSector = (data << 16);
        } else if (i == 118) {
            if (ata->bytesPerSector == 0) ata->bytesPerSector |= (data);
        }
    }
    return true;
}

/**
 * @brief Try to read a sector of an ATA storage device using LBA28 (28 bit addressing of sectors).
 * 
 * @param ata pointer to an ATA structure.
 * @param sector LBA (Logical Block Address) of the sector to read.
 * @param data pointer to a buffer to store the data from the sector.
 * @param count number of bytes to write into the buffer. (the maximum is the number of bytes per sector)
 * @return true the read operation was successful.
 * @return false the read operation failed.
 */
bool ATA_read28(ATA* ata, uint32_t sector, uint8_t* data, size_t count) {
    if (sector & 0xF0000000) return false;
    if (count > ata->bytesPerSector) return false;

    outportb(ata->portBase + ATADEVICEPORT, (ata->master ? 0xE0 : 0xF0) | ((sector & 0x0F000000) >> 24));
    outportb(ata->portBase + ATAERRORPORT, 0);
    outportb(ata->portBase + ATASECTORCOUNTPORT, 1);

    outportb(ata->portBase + ATALBALOWPORT, sector & 0x000000FF);
    outportb(ata->portBase + ATALBAMIDPORT, (sector & 0x0000FF00) >> 8);
    outportb(ata->portBase + ATALBAHIPORT, (sector & 0x00FF0000) >> 16);
    outportb(ata->portBase + ATACOMMANDPORT, 0x20);

    uint8_t status = inportb(ata->portBase + ATACOMMANDPORT);
    while (((status & 0x80) == 0x80) && ((status & 0x01) != 0x01))
        status = inportb(ata->portBase + ATACOMMANDPORT);

    if (status & 0x01) {
        puts("ATA: ERROR\n");
        return false;  //ERROR
    }

    for (uint16_t i = 0; i < count; i += 2) {
        uint16_t wdata = inportw(ata->portBase + ATADATAPORT);
        data[i] = wdata & 0x00FF;

        if (i + 1 < count)
            data[i + 1] = (wdata >> 8) & 0x00FF;
    }

    for (uint16_t i = count + (count % 2); i < ata->bytesPerSector; i += 2)
        inportw(ata->portBase + ATADATAPORT);

    return true;
}

void ATA_flush(ATA* ata) {
    outportb(ata->portBase + ATADEVICEPORT, ata->master ? 0xE0 : 0xF0);
    outportb(ata->portBase + ATACOMMANDPORT, 0xE7);

    uint8_t status = inportb(ata->portBase + ATACOMMANDPORT);
    while (((status & 0x80) == 0x80) && ((status & 0x01) != 0x01))
        status = inportb(ata->portBase + ATACOMMANDPORT);

    if (status & 0x01) {
        puts("ATA: ERROR\n");
        return;  //ERROR
    }
}

/**
 * @brief Try to write to a sector of an ATA storage device using LBA28 (28 bit addressing of sectors).
 * 
 * @param ata pointer to an ATA structure
 * @param sector LBA (Logical Block Address) of the sector to write to
 * @param data pointer to a buffer containing the data to write to the sector of the ATA storage device
 * @param count number of bytes to write to the storage device (The rest will get filled up with zeros)
 * @return true the write operation was successful.
 * @return false the write operation failed.
 */
bool ATA_write28(ATA* ata, uint32_t sector, uint8_t* data, size_t count) {
    if (sector & 0xF0000000) return false;
    if (count > ata->bytesPerSector) return false;

    outportb(ata->portBase + ATADEVICEPORT, (ata->master ? 0xE0 : 0xF0) | ((sector & 0x0F000000) >> 24));
    outportb(ata->portBase + ATAERRORPORT, 0);
    outportb(ata->portBase + ATASECTORCOUNTPORT, 1);

    outportb(ata->portBase + ATALBALOWPORT, sector & 0x000000FF);
    outportb(ata->portBase + ATALBAMIDPORT, (sector & 0x0000FF00) >> 8);
    outportb(ata->portBase + ATALBAHIPORT, (sector & 0x00FF0000) >> 16);
    outportb(ata->portBase + ATACOMMANDPORT, 0x30);

    //Polling
    //400ns delay
    inportb(ata->portBase + ATACOMMANDPORT);
    inportb(ata->portBase + ATACOMMANDPORT);
    inportb(ata->portBase + ATACOMMANDPORT);
    inportb(ata->portBase + ATACOMMANDPORT);

    uint8_t portval = inportb(ata->portBase + ATACOMMANDPORT);

    for (uint16_t j = 0; j < 3000; ++j) {
        if (!(portval & 0x80))
            break;

        sleepMilliSeconds(10);
        portval = inportb(ata->portBase + ATACOMMANDPORT);
    }

    for (uint16_t i = 0; i < count; i += 2) {
        uint16_t wdata = data[i];

        if (i + 1 < count)
            wdata |= ((uint16_t)data[i + 1]) << 8;

        outportw(ata->portBase + ATADATAPORT, wdata);
    }

    for (uint16_t i = count + (count % 2); i < ata->bytesPerSector; i += 2)
        outportw(ata->portBase + ATADATAPORT, 0x0000);

    //Flush
    ATA_flush(ata);
    return true;
}

/**
 * @brief Abstracted function for writing data to an ATA storage device.
 * 
 * @param ata pointer to an ATA structure
 * @param sector LBA (Logical Block Address) of the first sector to write to
 * @param data pointer to a buffer containing the data to write to the ATA storage device
 * @param count number of bytes to write to the storage device
 * @return true the write operation was successful.
 * @return false the write operation failed.
 */
bool ATA_write(ATA* ata, uint32_t sector, uint8_t* data, size_t count) {
    uint8_t i;
    for (i = 0; i < (count / 512); ++i)
        if (!ATA_write28(ata, sector + i, data + i * 512, 512))
            return false;

    if ((count - i * 512) > 0)
        if (!ATA_write28(ata, sector + i, data + i * 512, (count - i * 512)))
            return false;

    return true;
}

/**
 * @brief Abstracted function for reading data from an ATA storage device.
 * 
 * @param ata pointer to an ATA structure
 * @param sector LBA (Logical Block Address) of the first sector to read from
 * @param data pointer to a buffer to store the data
 * @param count number of bytes to read
 * @return true the read operation was successful.
 * @return false the read operation failed.
 */
bool ATA_read(ATA* ata, uint32_t sector, uint8_t* data, size_t count) {
    uint8_t i;
    for (i = 0; i < (count / 512); ++i)
        if (ATA_read28(ata, sector + i, data + i * 512, 512) == 0)
            return false;

    if ((count - i * 512) > 0)
        if (ATA_read28(ata, sector + i, data + i * 512, (count - i * 512)) == 0)
            return false;

    return true;
}

/**
 * @brief Create a new instance of ATA.
 * 
 * @param portBase controller IO port base
 * @param master Whether it is a master drive (true) or a slave drive (false)
 * @return ATA* a pointer to the new ATA structure
 */
ATA* ATA_create(uint16_t portBase, bool master) {
    ATA* result = (ATA*)malloc(sizeof(ATA), 0);
    result->bytesPerSector = 512;  //default
    result->portBase = portBase;
    result->master = master;
    ATA_identify(result);
    return result;
}

/**
 * @brief Free an instance of ATA from memory.
 * 
 * @param ata pointer to the ATA structure
 */
void ATA_destroy(ATA* ata) {
    if (ata)
        free(ata);
}
