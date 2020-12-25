#include "storage_devManager.h"

#include "ata.h"
#include "cmos.h"
#include "flpydsk.h"
#include "partitionManager.h"

static listHead_t* devList;
uint32_t bootVolID;

void storage_devManager_init() {
    uint32_t* bootVolIDPtr = (uint32_t*)0x0FFA;
    bootVolID = bootVolIDPtr[0];
    devList = list_create();

    //Check for the floppy drives
    uint8_t tmp = cmos_read(0x10);
    //Checking for master floppy (base: 0x3F0)
    switch (tmp >> 4) {
        case 1:  //360 KB 5.25 Drive
            break;
        case 2:  //1.2 MB 5.25 Drive
            break;
        case 3:  //720 KB 3.5 Drive
            break;
        case 4:  //1.44 MB 3.5 Drive
            ;
            storage_dev_t* flpyDev = malloc(sizeof(storage_dev_t), 0);
            flpyDev->type = STORAGE_DEVMANAGER_TYPE_FLOPPY;
            Flpydsk_t* flpydsk = flpydsk_create(0);
            flpyDev->inst = flpydsk;
            flpyDev->readSector = (void*)flpydsk_read_sectors;
            flpyDev->writeSector = (void*)flpydsk_write_sectors;
            flpyDev->partitions = NULL;
            uint32_t retBootPart = PartManage_analyzeDev(flpyDev);
            if (retBootPart) {
                ODA.bootPart = retBootPart - 1;
                ODA.bootDev = list_getSize(devList);
            }
            list_append(devList, flpyDev);
            break;
        case 5:  //2.88 MB 3.5 Drive
            break;
    }
    //Checking for slave floppy (base: 0x370)
    switch (tmp & 0x0F) {
        case 1:  //360 KB 5.25 Drive
            break;
        case 2:  //1.2 MB 5.25 Drive
            break;
        case 3:  //720 KB 3.5 Drive
            break;
        case 4:  //1.44 MB 3.5 Drive
            ;
            storage_dev_t* flpyDev = malloc(sizeof(storage_dev_t), 0);
            flpyDev->type = STORAGE_DEVMANAGER_TYPE_FLOPPY;
            Flpydsk_t* flpydsk = flpydsk_create(1);
            flpyDev->inst = flpydsk;
            flpyDev->readSector = (void*)flpydsk_read_sectors;
            flpyDev->writeSector = (void*)flpydsk_write_sectors;
            flpyDev->partitions = NULL;
            uint32_t retBootPart = PartManage_analyzeDev(flpyDev);
            if (retBootPart) {
                ODA.bootPart = retBootPart - 1;
                ODA.bootDev = list_getSize(devList);
            }
            list_append(devList, flpyDev);
            break;
        case 5:  //2.88 MB 3.5 Drive
            break;
    }

    //Check for IDE hard drives
    ATA* ata0m = ATA_create(0x1F0, 1);  //Primary master
    if (ATA_identify(ata0m)) {
        storage_dev_t* devAta = malloc(sizeof(storage_dev_t), 0);
        devAta->type = STORAGE_DEVMANAGER_TYPE_HDD;
        devAta->inst = ata0m;
        devAta->readSector = (void*)ATA_read;
        devAta->writeSector = (void*)ATA_write;
        devAta->partitions = NULL;
        uint32_t retBootPart = PartManage_analyzeDev(devAta);
        if (retBootPart) {
            ODA.bootPart = retBootPart - 1;
            ODA.bootDev = list_getSize(devList);
        }
        list_append(devList, devAta);
    } else {
        ATA_destroy(ata0m);
    }
    ATA* ata0s = ATA_create(0x1F0, 0);  //Primary slave
    if (ATA_identify(ata0s)) {
        storage_dev_t* devAta = malloc(sizeof(storage_dev_t), 0);
        devAta->type = STORAGE_DEVMANAGER_TYPE_HDD;
        devAta->inst = ata0s;
        devAta->readSector = (void*)ATA_read;
        devAta->writeSector = (void*)ATA_write;
        devAta->partitions = NULL;
        uint32_t retBootPart = PartManage_analyzeDev(devAta);
        if (retBootPart) {
            ODA.bootPart = retBootPart - 1;
            ODA.bootDev = list_getSize(devList);
        }
        list_append(devList, devAta);
    } else {
        ATA_destroy(ata0s);
    }
    ATA* ata1m = ATA_create(0x170, 1);  //Secondary master
    if (ATA_identify(ata1m)) {
        storage_dev_t* devAta = malloc(sizeof(storage_dev_t), 0);
        devAta->type = STORAGE_DEVMANAGER_TYPE_HDD;
        devAta->inst = ata1m;
        devAta->readSector = (void*)ATA_read;
        devAta->writeSector = (void*)ATA_write;
        devAta->partitions = NULL;
        uint32_t retBootPart = PartManage_analyzeDev(devAta);
        if (retBootPart) {
            ODA.bootPart = retBootPart - 1;
            ODA.bootDev = list_getSize(devList);
        }
        list_append(devList, devAta);
    } else {
        ATA_destroy(ata1m);
    }
    ATA* ata1s = ATA_create(0x170, 0);  //Secondary slave
    if (ATA_identify(ata1s)) {
        storage_dev_t* devAta = malloc(sizeof(storage_dev_t), 0);
        devAta->type = STORAGE_DEVMANAGER_TYPE_HDD;
        devAta->inst = ata1s;
        devAta->readSector = (void*)ATA_read;
        devAta->writeSector = (void*)ATA_write;
        devAta->partitions = NULL;
        uint32_t retBootPart = PartManage_analyzeDev(devAta);
        if (retBootPart) {
            ODA.bootPart = retBootPart - 1;
            ODA.bootDev = list_getSize(devList);
        }
        list_append(devList, devAta);
    } else {
        ATA_destroy(ata1s);
    }

    //TODO: Detect PCI devices
}

listHead_t* storage_getDevList() {
    return devList;
}

void storage_readSector(storage_dev_t* dev, uint32_t sector, uint8_t* data, size_t count) {
    dev->readSector(dev->inst, sector, data, count);
}

void storage_writeSector(storage_dev_t* dev, uint32_t sector, uint8_t* data, size_t count) {
    dev->writeSector(dev->inst, sector, data, count);
}
