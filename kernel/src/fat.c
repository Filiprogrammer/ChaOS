#include "fat.h"
#include "list.h"
#include "paging.h"
#include "math.h"
#include "string.h"

FAT* FAT_create(storage_dev_t* storageDev, uint32_t lbaStart, uint32_t lbaLength, uint8_t fileSys) {
    FAT* inst = (FAT*) malloc(sizeof(FAT), 0);
    inst->storageDev = storageDev;
    inst->lbaStart = lbaStart;
    inst->lbaLength = lbaLength;
    inst->fileSys = fileSys;
    //Read BIOS Parameter Block
    uint8_t secBytes[512];
    storage_readSector(storageDev, lbaStart, secBytes, 512);
    switch(fileSys){
        case FAT12: // FAT12
            inst->sectorsPerFAT = secBytes[0x16] + (secBytes[0x17] << 8);
            inst->startClustOfRootDir = 0; // unused in FAT12
            inst->maxRootDirEntries = secBytes[17] + (secBytes[18] << 8);
            break;
        case FAT16: // FAT16
            inst->sectorsPerFAT = secBytes[0x16] + (secBytes[0x17] << 8);
            inst->startClustOfRootDir = 0; // unused in FAT16
            inst->maxRootDirEntries = secBytes[17] + (secBytes[18] << 8);
            break;
        default: // FAT32
            inst->sectorsPerFAT = secBytes[0x24] + (secBytes[0x25] << 8) + (secBytes[0x26] << 16) + (secBytes[0x27] << 24);
            inst->startClustOfRootDir = secBytes[0x2C] + (secBytes[0x2D] << 8) + (secBytes[0x2E] << 16) + (secBytes[0x2F] << 24);
            inst->maxRootDirEntries = 0; // unused in FAT32 (number of root directory entries is not limited)
            break;
    }
    inst->reservedSectors = secBytes[14] + (secBytes[15] << 8);
    inst->FATCopies = secBytes[16];
    inst->sectorsPerCluster = secBytes[13];
    return inst;
}

void FAT_destroy(FAT* inst) {
    if (inst) {
        free(inst);
    }
}

uint32_t FAT_FATgetClusterDetailed(FAT* inst, uint32_t start_clust){
    if (start_clust < 2) return 0;
    switch(inst->fileSys){
        case FAT12: // FAT12
            if (start_clust >= ((inst->sectorsPerFAT*512*2)/3)) return 0;
            break;
        case FAT16: // FAT16
            if (start_clust >= inst->sectorsPerFAT*256) return 0;
            break;
        default: // FAT32
            if (start_clust >= inst->sectorsPerFAT*128) return 0;
            break;
    }
    uint8_t secBytes[512];
    switch(inst->fileSys){
        case FAT12: // FAT12
            if(((start_clust * 3) / (512*2)) == (((start_clust*3) / 2 + 1) / 512)){
                // One sector is used
                storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + (start_clust * 3) / (512*2), secBytes, 512);
                if ((start_clust % 2) == 0) { // begins at start of byte
                    return (secBytes[((start_clust*3)/2) % 512] + ((secBytes[((start_clust*3)/2 + 1) % 512] & 0x0F) << 8));
                } else {
                    return ((secBytes[((start_clust*3)/2 + 1) % 512] << 4) + ((secBytes[((start_clust*3)/2) % 512] & 0xF0) >> 4));
                }
            } else {
                // Two sectors are used
                storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + (start_clust * 3) / (512*2), secBytes, 512);
                uint8_t secBytes2[512];
                storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + (start_clust * 3 + 2) / (512*2), secBytes2, 512);
                if ((start_clust % 2) == 0) { // begins at start of byte
                    return (secBytes[511] + ((secBytes2[0] & 0x0F) << 8));
                } else {
                    return ((secBytes2[0] << 4) + ((secBytes[511] & 0xF0) >> 4));
                }
            }
            break;
        case FAT16: // FAT16
            storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + start_clust/256, secBytes, 512);
            return secBytes[(start_clust*2) % 512] + (secBytes[((start_clust * 2) % 512) + 1] << 8);
            break;
        default: // FAT32
            storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + start_clust/128, secBytes, 512);
            return secBytes[(start_clust*4) % 512] + (secBytes[((start_clust * 4) % 512) + 1] << 8) + (secBytes[((start_clust * 4) % 512) + 2] << 16) + (secBytes[((start_clust * 4) % 512) + 3] << 24);
            break;
    }
    return 0;
    /*if (start_clust < 2) return 0;
    if (start_clust >= inst->sectorsPerFAT*128) return 0;
    uint8_t secBytes[512];
    storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + start_clust/128, secBytes, 512);
    uint32_t ret_clust = secBytes[(start_clust*4) % 512] + (secBytes[((start_clust * 4) % 512) + 1] << 8) + (secBytes[((start_clust * 4) % 512) + 2] << 16) + (secBytes[((start_clust * 4) % 512) + 3] << 24);
    if (ret_clust == 0) return 0; //Free cluster
    if (ret_clust == 0x0FFFFFF7) return 0x0FFFFFF7; //Damaged cluster
    if (ret_clust >= 0x0FFFFFF8) return 0x0FFFFFF8; //Last cluster
    return ret_clust;*/
}

uint32_t FAT_FATgetCluster(FAT* inst, uint32_t start_clust){
    uint32_t ret_clust = FAT_FATgetClusterDetailed(inst, start_clust);
    if (ret_clust == 0) return 0; //Free cluster
    switch(inst->fileSys){
        case FAT12:
            if (ret_clust == 0xFF7) return 0; //Damaged cluster
            if (ret_clust >= 0xFF8) return 0; //Last cluster
            break;
        case FAT16:
            if (ret_clust == 0xFFF7) return 0; //Damaged cluster
            if (ret_clust >= 0xFFF8) return 0; //Last cluster
            break;
        default:
            if (ret_clust == 0x0FFFFFF7) return 0; //Damaged cluster
            if (ret_clust >= 0x0FFFFFF8) return 0; //Last cluster
            break;
    }
    return ret_clust;
}

uint32_t* FAT_FATgetClusterChain(FAT* inst, uint32_t start_clust, uint32_t start_offset, uint32_t max_size){
    /*listHead_t* clustList = listCreate();
    uint32_t i = 0;
    while(start_clust != 0){
        if(i >= (start_offset/(inst->sectorsPerCluster*512))){
            listAppend(clustList, (void*)start_clust);
        }
        start_clust = FAT_FATgetCluster(inst, start_clust);
        if(++i >= (start_offset+max_size+inst->sectorsPerCluster*512-1) / (inst->sectorsPerCluster*512)){
            break;
        }
    }
    return clustList;*/
    
    uint32_t* clustArr = malloc((max_size*4)/(inst->sectorsPerCluster*512)+4, 0);
    //listHead_t* clustList = listCreate();
    uint32_t i = 0;
    uint32_t prev_clust = start_clust+131072; // Just make sure, that prev_clust starts with a different value than start_clust
    uint8_t secBytes[512];
    
    while(start_clust != 0){
        if(i >= (start_offset/(inst->sectorsPerCluster*512))){
            clustArr[1+i-(start_offset/(inst->sectorsPerCluster*512))] = start_clust;
            //listAppend(clustList, (void*)start_clust);
        }
        
        switch(inst->fileSys){
            case FAT12: // FAT12
                if(((start_clust * 3) / (512*2)) == (((start_clust*3) / 2 + 1) / 512)){
                    // One sector is used
                    if(((prev_clust * 3) / (512*2)) != ((start_clust * 3) / (512*2))){
                        storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + ((start_clust * 3) / (512*2)), secBytes, 512);
                    }
                    prev_clust = start_clust;
                    if ((start_clust % 2) == 0) { // begins at start of byte
                        start_clust = (secBytes[((start_clust*3)/2) % 512] + ((secBytes[((start_clust*3)/2 + 1) % 512] & 0x0F) << 8));
                    } else {
                        start_clust = ((secBytes[((start_clust*3)/2 + 1) % 512] << 4) + ((secBytes[((start_clust*3)/2) % 512] & 0xF0) >> 4));
                    }
                } else {
                    // Two sectors are used
                    if(((prev_clust * 3) / (512*2)) != ((start_clust * 3) / (512*2))){
                        storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + ((start_clust * 3) / (512*2)), secBytes, 512);
                    }
                    uint8_t secBytes2[512];
                    storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + ((start_clust*3)/2 + 1) / 512, secBytes2, 512);
                    prev_clust = start_clust;
                    if ((start_clust % 2) == 0) { // begins at start of byte
                        start_clust = (secBytes[511] + ((secBytes2[0] & 0x0F) << 8));
                    } else {
                        start_clust = ((secBytes2[0] << 4) + ((secBytes[511] & 0xF0) >> 4));
                    }
                }
                break;
            case FAT16: // FAT16
                if((prev_clust/256) != (start_clust/256)){
                    storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + start_clust/256, secBytes, 512);
                }
                prev_clust = start_clust;
                start_clust = secBytes[(start_clust*2) % 512] + (secBytes[((start_clust * 2) % 512) + 1] << 8);
                break;
            default: // FAT32
                if((prev_clust/128) != (start_clust/128)){
                    storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + start_clust/128, secBytes, 512);
                }
                prev_clust = start_clust;
                start_clust = secBytes[(start_clust*4) % 512] + (secBytes[((start_clust * 4) % 512) + 1] << 8) + (secBytes[((start_clust * 4) % 512) + 2] << 16) + (secBytes[((start_clust * 4) % 512) + 3] << 24);
                break;
        }
        
        if(++i >= (start_offset+max_size+inst->sectorsPerCluster*512-1) / (inst->sectorsPerCluster*512)){
            break;
        }
    }
    
    //return clustList;
    clustArr[0] = i-(start_offset/(inst->sectorsPerCluster*512));
    return clustArr;
}

void FAT_FATclearClusterChain(FAT* inst, uint32_t start_clust){
    uint32_t prev_clust = start_clust+131072; // Just make sure, that prev_clust starts with a different value than start_clust
    uint8_t secBytes[512];
    
    while(start_clust != 0){
        switch(inst->fileSys){
            case FAT12: // FAT12
                if(((uint32_t)(start_clust * 1.5) / 512) == ((uint32_t)(start_clust * 1.5 + 1) / 512)){
                    // One sector is used
                    if(((uint32_t)(prev_clust * 1.5) / 512) != ((uint32_t)(start_clust * 1.5) / 512)){
                        storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + (uint32_t)(start_clust * 1.5/512), secBytes, 512);
                    }
                    prev_clust = start_clust;
                    if ((uint32_t)(start_clust*1.5) == start_clust*1.5) { // begins at start of byte
                        start_clust = (secBytes[(uint32_t)(start_clust*1.5) % 512] + ((secBytes[(uint32_t)(start_clust*1.5 + 1) % 512] & 0x0F) << 8));
                        secBytes[(uint32_t)(prev_clust*1.5) % 512] = 0;
                        secBytes[(uint32_t)(prev_clust*1.5 + 1) % 512] = secBytes[(uint32_t)(prev_clust*1.5 + 1) % 512] & 0xF0;
                    } else {
                        start_clust = ((secBytes[(uint32_t)(start_clust*1.5 + 1) % 512] << 4) + ((secBytes[(uint32_t)(start_clust*1.5) % 512] & 0xF0) >> 4));
                        secBytes[(uint32_t)(prev_clust*1.5 + 1) % 512] = 0;
                        secBytes[(uint32_t)(prev_clust*1.5) % 512] = secBytes[(uint32_t)(prev_clust*1.5) % 512] & 0x0F;
                    }
                    if(((uint32_t)(prev_clust * 1.5) / 512) != ((uint32_t)(start_clust * 1.5) / 512)){
                        storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + (uint32_t)(prev_clust * 1.5) / 512, secBytes, 512);
                    }
                } else {
                    // Two sectors are used
                    if(((uint32_t)(prev_clust * 1.5) / 512) != ((uint32_t)(start_clust * 1.5) / 512)){
                        storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + (uint32_t)(start_clust * 1.5) / 512, secBytes, 512);
                    }
                    uint8_t secBytes2[512];
                    storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + (uint32_t)(start_clust * 1.5 + 1) / 512, secBytes2, 512);
                    prev_clust = start_clust;
                    if ((uint32_t)(start_clust*1.5) == start_clust*1.5) { // begins at start of byte
                        start_clust = (secBytes[511] + ((secBytes2[0] & 0x0F) << 8));
                        secBytes[511] = 0;
                        secBytes2[0] = secBytes2[0] & 0xF0;
                    } else {
                        start_clust = ((secBytes2[0] << 4) + ((secBytes[511] & 0xF0) >> 4));
                        secBytes[511] = secBytes[511] & 0x0F;
                        secBytes2[0] = 0;
                    }
                    storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + (uint32_t)(prev_clust * 1.5) / 512, secBytes, 512);
                    storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + (uint32_t)(prev_clust * 1.5 + 1) / 512, secBytes2, 512);
                }
                if (start_clust == 0xFF7) start_clust = 0; //Damaged cluster
                if (start_clust >= 0xFF8) start_clust = 0; //Last cluster
                break;
            case FAT16: // FAT16
                if((prev_clust/256) != (start_clust/256)){
                    storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + start_clust/256, secBytes, 512);
                }
                prev_clust = start_clust;
                start_clust = secBytes[(start_clust*2) % 512] + (secBytes[((start_clust * 2) % 512) + 1] << 8);
                if (start_clust == 0xFFF7) start_clust = 0; //Damaged cluster
                if (start_clust >= 0xFFF8) start_clust = 0; //Last cluster
                secBytes[(prev_clust*2) % 512] = 0;
                secBytes[((prev_clust * 2) % 512) + 1] = 0;
                if((prev_clust/256) != (start_clust/256)){
                    storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + prev_clust/256, secBytes, 512);
                }
                break;
            default: // FAT32
                if((prev_clust/128) != (start_clust/128)){
                    storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + start_clust/128, secBytes, 512);
                }
                prev_clust = start_clust;
                start_clust = secBytes[(start_clust*4) % 512] + (secBytes[((start_clust * 4) % 512) + 1] << 8) + (secBytes[((start_clust * 4) % 512) + 2] << 16) + (secBytes[((start_clust * 4) % 512) + 3] << 24);
                if (start_clust == 0x0FFFFFF7) start_clust = 0; //Damaged cluster
                if (start_clust >= 0x0FFFFFF8) start_clust = 0; //Last cluster
                secBytes[(prev_clust*4) % 512] = 0;
                secBytes[((prev_clust * 4) % 512) + 1] = 0;
                secBytes[((prev_clust * 4) % 512) + 2] = 0;
                secBytes[((prev_clust * 4) % 512) + 3] = 0;
                if((prev_clust/128) != (start_clust/128)){
                    storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + prev_clust/128, secBytes, 512);
                }
                break;
        }
    }
    switch(inst->fileSys){
        case FAT12: // FAT12
            storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + (uint32_t)(prev_clust * 1.5/512), secBytes, 512);
            break;
        case FAT16: // FAT16
            storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + prev_clust/256, secBytes, 512);
            break;
        default: // FAT32
            storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + prev_clust/128, secBytes, 512);
            break;
    }
}

void FAT_FATsetCluster(FAT* inst, uint32_t start_clust, uint32_t pointing_to){
    switch(inst->fileSys){
        case FAT12: // FAT12
            if (start_clust >= (uint32_t)((inst->sectorsPerFAT*512)/1.5)) return;
            break;
        case FAT16: // FAT16
            if (start_clust >= inst->sectorsPerFAT*256) return;
            break;
        default: // FAT32
            if (start_clust >= inst->sectorsPerFAT*128) return;
            break;
    }
    uint8_t secBytes[512];
    switch(inst->fileSys){
        case FAT12: // FAT12
            if(((uint32_t)(start_clust * 1.5) / 512) == ((uint32_t)(start_clust * 1.5 + 1) / 512)){
                // One sector is used
                storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + (uint32_t)(start_clust * 1.5/512), secBytes, 512);
                if ((uint32_t)(start_clust*1.5) == start_clust*1.5) { // begins at start of byte
                    secBytes[(uint32_t)(start_clust*1.5) % 512] = (uint8_t)(pointing_to & 0x0FF);
                    secBytes[(uint32_t)(start_clust*1.5 + 1) % 512] &= 0xF0;
                    secBytes[(uint32_t)(start_clust*1.5 + 1) % 512] |= (uint8_t)((pointing_to & 0xF00) >> 8);
                } else {
                    secBytes[(uint32_t)(start_clust*1.5) % 512] &= 0x0F;
                    secBytes[(uint32_t)(start_clust*1.5) % 512] |= (uint8_t)((pointing_to & 0x0F) << 4);
                    secBytes[(uint32_t)(start_clust*1.5 + 1) % 512] = (uint8_t)(pointing_to >> 4);
                }
                storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + (uint32_t)(start_clust * 1.5/512), secBytes, 512);
            } else {
                // Two sectors are used
                storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + (uint32_t)(start_clust * 1.5) / 512, secBytes, 512);
                uint8_t secBytes2[512];
                storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + (uint32_t)(start_clust * 1.5 + 1) / 512, secBytes2, 512);
                if ((uint32_t)(start_clust*1.5) == start_clust*1.5) { // begins at start of byte
                    secBytes[511] = (uint8_t)(pointing_to & 0x0FF);
                    secBytes2[0] &= 0xF0;
                    secBytes2[0] |= (uint8_t)((pointing_to & 0xF00) >> 8);
                } else {
                    secBytes[511] &= 0x0F;
                    secBytes[511] |= (uint8_t)((pointing_to & 0x0F) << 4);
                    secBytes2[0] = (uint8_t)(pointing_to >> 4);
                }
                storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + (uint32_t)(start_clust * 1.5/512), secBytes, 512);
                storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + (uint32_t)(start_clust * 1.5 + 1) / 512, secBytes2, 512);
            }
            break;
        case FAT16: // FAT16
            storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + start_clust/256, secBytes, 512);
            secBytes[(start_clust * 2) % 512] = (uint8_t)(pointing_to & 0x00FF);
            secBytes[(start_clust * 2) % 512 + 1] = (uint8_t)((pointing_to & 0xFF00) >> 8);
            storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + start_clust/256, secBytes, 512);
            break;
        default: // FAT32
            storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + start_clust/128, secBytes, 512);
            secBytes[(start_clust * 4) % 512] = (uint8_t)(pointing_to & 0x000000FF);
            secBytes[(start_clust * 4) % 512 + 1] = (uint8_t)((pointing_to & 0x0000FF00) >> 8);
            secBytes[(start_clust * 4) % 512 + 2] = (uint8_t)((pointing_to & 0x00FF0000) >> 16);
            secBytes[(start_clust * 4) % 512 + 3] = (uint8_t)((pointing_to & 0xFF000000) >> 24);
            storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + start_clust/128, secBytes, 512);
            break;
    }
    
    /*if (start_clust >= inst->sectorsPerFAT*128) return;
    uint8_t secBytes[512];
    storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + start_clust/128, secBytes, 512);
    secBytes[(start_clust * 4) % 512] = (uint8_t)(pointing_to & 0x000000FF);
    secBytes[(start_clust * 4) % 512 + 1] = (uint8_t)((pointing_to & 0x0000FF00) >> 8);
    secBytes[(start_clust * 4) % 512 + 2] = (uint8_t)((pointing_to & 0x00FF0000) >> 16);
    secBytes[(start_clust * 4) % 512 + 3] = (uint8_t)((pointing_to & 0xFF000000) >> 24);
    storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + start_clust / 128, secBytes, 512);*/
}

uint32_t FAT_FATfindFreeCluster(FAT* inst){
    uint8_t secBytes[512];
    uint32_t cur_clust; //Starts at cluster 2, which is the first one
    uint32_t prev_fatsec = 0xFFFFFFFF; // Has to be different than cur_clust
    uint32_t ret_clust;
    switch(inst->fileSys){
        case FAT12: // FAT12
            for(cur_clust = 2; cur_clust < (uint32_t)((inst->sectorsPerFAT*512)/1.5); ++cur_clust){
                if(((uint32_t)(cur_clust * 1.5) / 512) == ((uint32_t)(cur_clust * 1.5 + 1) / 512)){
                    // One sector is used
                    if(prev_fatsec != (uint32_t)(cur_clust * 1.5/512)){ // Make sure the sector is only beeing read when necessary
                        storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + (uint32_t)(cur_clust * 1.5/512), secBytes, 512);
                        prev_fatsec = (uint32_t)(cur_clust * 1.5/512);
                    }
                    if ((uint32_t)(cur_clust*1.5) == cur_clust*1.5) { // begins at start of byte
                        ret_clust = (secBytes[(uint32_t)(cur_clust*1.5) % 512] + ((secBytes[(uint32_t)(cur_clust*1.5 + 1) % 512] & 0x0F) << 8));
                    } else {
                        ret_clust = ((secBytes[(uint32_t)(cur_clust*1.5 + 1) % 512] << 4) + ((secBytes[(uint32_t)(cur_clust*1.5) % 512] & 0xF0) >> 4));
                    }
                } else {
                    // Two sectors are used
                    storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + (uint32_t)(cur_clust * 1.5) / 512, secBytes, 512);
                    uint8_t secBytes2[512];
                    storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + (uint32_t)(cur_clust * 1.5 + 1) / 512, secBytes2, 512);
                    if ((uint32_t)(cur_clust*1.5) == cur_clust*1.5) { // begins at start of byte
                        ret_clust = (secBytes[511] + ((secBytes2[0] & 0x0F) << 8));
                    } else {
                        ret_clust = ((secBytes2[0] << 4) + ((secBytes[511] & 0xF0) >> 4));
                    }
                }
                if(ret_clust == 0) return cur_clust;
            }
            break;
        case FAT16: // FAT16
            for(cur_clust = 2; cur_clust < (inst->sectorsPerFAT*256); ++cur_clust){
                if(prev_fatsec != cur_clust/256){
                    storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + cur_clust/256, secBytes, 512);
                    prev_fatsec = cur_clust/256;
                }
                ret_clust = secBytes[(cur_clust * 2) % 512] + (secBytes[((cur_clust * 2) % 512)+1] << 8);
                if(ret_clust == 0) return cur_clust; //Free cluster
            }
            break;
        default: // FAT32
            for(cur_clust = 2; cur_clust < (inst->sectorsPerFAT*128); ++cur_clust){
                if(prev_fatsec != cur_clust/128){
                    storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + cur_clust/128, secBytes, 512);
                    prev_fatsec = cur_clust/128;
                }
                ret_clust = secBytes[(cur_clust * 4) % 512] + (secBytes[((cur_clust * 4) % 512)+1] << 8) + (secBytes[((cur_clust * 4) % 512)+2] << 16) + (secBytes[((cur_clust * 4) % 512)+3] << 24);
                if(ret_clust == 0) return cur_clust; //Free cluster
            }
            break;
    }
    return 0;
}

char* formatFilePath(char* filepath){
    char delimiter[] = "/\\";
    #define FAT_MAX_PATH_FILES 16
    #define FAT_MAX_FNAME_LEN 34
    char spl_path[FAT_MAX_PATH_FILES][FAT_MAX_FNAME_LEN];
    unsigned int ret_arr_size = strlen(filepath)+1;
    int i, j;
    for(i = 0; i < FAT_MAX_PATH_FILES; ++i){
        for(j = 0; j < FAT_MAX_FNAME_LEN; ++j){
            spl_path[i][j] = '\0';
        }
    }
    int path_size = 0;
    char* cur_file = strtok(filepath, delimiter);
    while(cur_file != 0 && path_size < FAT_MAX_PATH_FILES){
        if(strlen(cur_file) > FAT_MAX_FNAME_LEN-1) break; //make sure current filename is not to long
        strcpy(spl_path[path_size], cur_file);
        cur_file = strtok(0, delimiter);
        ++path_size;
    }
    for(i = 0; i < path_size; ++i){
        if(strcmp(spl_path[i], ".") == 0){ //if spl_path[i] equals "."
            strcpy(spl_path[i], "");
            continue;
        }
        if(strcmp(spl_path[i], "..") == 0){ //if spl_path[i] equals ".."
            if(i > 0){
                for(j = i-1; j >= 0; --j){
                    if(strlen(spl_path[j]) != 0){
                        strcpy(spl_path[j], "");
                        break;
                    }
                }
            }
            strcpy(spl_path[i], "");
            continue;
        }
    }
    
    for(i = 0; i < ret_arr_size; ++i){
        filepath[i] = '\0';
    }
    for(i = 0; i < path_size; ++i){
        if(strcmp(spl_path[i], "")){ //if spl_path[i] does NOT equal to ""
            if(strcmp(filepath, "") == 0){ //if filepath.Length is 0
                strcpy(filepath, spl_path[i]);
            }
            else{
                strcat(filepath, "/");
                strcat(filepath, spl_path[i]);
            }
        }
    }
    return filepath;
}

uint32_t* FAT_readFileEntryLocation(FAT* inst, char* filepath){
    uint32_t ret_long_clust = 0;
    uint32_t ret_long_offset = 0;
    uint8_t ret_has_long = 0;
    uint8_t ret[32];
    //Format and split the file path
    char spl_path[FAT_MAX_PATH_FILES][FAT_MAX_FNAME_LEN];
    for(uint32_t k = 0; k < FAT_MAX_PATH_FILES; ++k){
        for(uint32_t l = 0; l < FAT_MAX_FNAME_LEN; ++l){
            spl_path[k][l] = 0;
        }
    }
    char delimiter[] = "/";
    int path_size = 0;
    formatFilePath(filepath);
    size_t tmp_filepath_len = strlen(filepath)+1;
    char tmp_filepath[tmp_filepath_len];
    memset(tmp_filepath, 0, tmp_filepath_len);
    strcpy(tmp_filepath, filepath);
    char* cur_file = strtok(tmp_filepath, delimiter);
    while(cur_file != 0 && path_size < FAT_MAX_PATH_FILES){
        if(strlen(cur_file) > FAT_MAX_FNAME_LEN-1) break; //make sure current filename is not to long
        strcpy(spl_path[path_size], cur_file);
        cur_file = strtok(0, delimiter);
        ++path_size;
    }
    
    uint32_t cur_clust = inst->startClustOfRootDir;
    
    for(uint32_t k = 0; k < path_size; ++k){
        //Split the current file into name and extension
        int32_t cur_file_len = strlen(spl_path[k]);
        int32_t lastDot = strrchr(spl_path[k], '.');
        if(lastDot == -1) lastDot = cur_file_len;
        char cur_file_name[lastDot+1];
        for(uint32_t j = 0; j < lastDot; ++j){
            cur_file_name[j] = spl_path[k][j];
        }
        cur_file_name[lastDot] = 0; //Null termination
        char cur_file_ext[cur_file_len-lastDot];
        for(uint32_t j = (lastDot+1); j < cur_file_len; ++j){
            cur_file_ext[j-(lastDot+1)] = spl_path[k][j];
        }
        cur_file_ext[MAX(cur_file_len-lastDot-1, 0)] = 0; //Null termination
        
        char long_fname_buffer[208];
        //Clear long_fname_buffer
        for(uint8_t l = 0; l < 208; ++l){
            long_fname_buffer[l] = 0;
        }
        
        
        
        
        
        
        
        
        
        if((inst->fileSys <= FAT16) && k == 0){ //root directory of FAT12 and FAT16
            uint8_t loopBreak = 0;
            for(uint32_t j = 0; j < inst->maxRootDirEntries/16; ++j){
                uint8_t cur_sec_bytes[512];
                storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + j, cur_sec_bytes, 512);
                for(uint32_t i = 0; i < 512; i += 32){
                    if(cur_sec_bytes[i] == 0x00) return 0; //Free from here on
                    if(cur_sec_bytes[i] == 0xE5) continue; //File was deleted
                    
                    if(cur_sec_bytes[i+11] == 0x0F) { //Part of long filename
                        if(!ret_has_long){
                            ret_long_clust = j;
                            ret_long_offset = i;
                            ret_has_long = 1;
                        }
                        
                        uint8_t cur_long_fname_temp[] = {cur_sec_bytes[i + 1], cur_sec_bytes[i + 3], cur_sec_bytes[i + 5], cur_sec_bytes[i + 7], cur_sec_bytes[i + 9],
                                    cur_sec_bytes[i + 14], cur_sec_bytes[i + 16], cur_sec_bytes[i + 18], cur_sec_bytes[i + 20], cur_sec_bytes[i + 22], cur_sec_bytes[i + 24],
                                    cur_sec_bytes[i + 28], cur_sec_bytes[i + 30]};
                        for(uint8_t l = 0; l < /*Length of cur_long_fname_temp*/13; ++l){
                            if ((/*if last long file name entry*/(cur_sec_bytes[i] & 0xF0) == 0x40) && (cur_long_fname_temp[l] == 0)) break;
                            long_fname_buffer[((cur_sec_bytes[i] & 0x0F)-1)*13+l] = (char)cur_long_fname_temp[l];
                        }
                        continue;
                    }
                    
                    char cur_filename[FAT_MAX_FNAME_LEN];
                    char cur_ext[FAT_MAX_FNAME_LEN];
                    
                    if(long_fname_buffer[0] == 0){ //if file name is short
                        for(uint8_t l = 0; l < 8; ++l){
                            cur_filename[l] = (char)cur_sec_bytes[i+l];
                        }
                        cur_filename[8] = 0; //Null termination
                        strtrimend(cur_filename);
                        for(uint8_t l = 0; l < 3; ++l){
                            cur_ext[l] = (char)cur_sec_bytes[i+8+l];
                        }
                        cur_ext[3] = 0; //Null termination
                        strtrimend(cur_ext);
                        ret_has_long = 0;
                    }else{ //if file name is long
                        //Split the current file into name and extension
                        cur_file_len = strlen(long_fname_buffer);
                        lastDot = strrchr(long_fname_buffer, '.');
                        if(lastDot == -1) lastDot = cur_file_len;
                        cur_filename[1] = 0; //Null termination if lastDot = 0
                        for(uint32_t l = 0; l < lastDot; ++l){
                            cur_filename[l] = long_fname_buffer[l];
                        }
                        cur_filename[lastDot] = 0; //Null termination
                        for(uint32_t l = (lastDot+1); l < cur_file_len; ++l){
                            cur_ext[l-(lastDot+1)] = long_fname_buffer[l];
                        }
                        cur_ext[cur_file_len-lastDot-1] = 0; //Null termination
                        
                        //Clear long_fname_buffer
                        for(uint8_t l = 0; l < 208; ++l){
                            long_fname_buffer[l] = 0;
                        }
                    }
                    
                    tolower(cur_filename);
                    tolower(cur_file_name);
                    if(strcmp(cur_filename, cur_file_name) == 0){ //if the requested file name matches the file name of the current entry
                        if(*cur_file_ext == 0){ //if cur_file_ext is empty
                            //Copy the current file entry to ret
                            for(uint8_t l = 0; l < 32; ++l){
                                ret[l] = cur_sec_bytes[i+l];
                            }
                        }else{
                            tolower(cur_ext);
                            tolower(cur_file_ext);
                            if(strcmp(cur_ext, cur_file_ext) == 0){ //if the requested file extension matches the file extension of the current entry
                                //Copy the current file entry to ret
                                for(uint8_t l = 0; l < 32; ++l){
                                    ret[l] = cur_sec_bytes[i+l];
                                }
                            }
                            else{
                                ret_has_long = 0;
                                continue;
                            }
                        }
                        
                        if(k == (path_size-1)){ //if is final file
                            uint32_t* entry_loc = malloc(16, 0);
                            entry_loc[0] = j;
                            entry_loc[1] = i;
                            if(ret_has_long) {
                                entry_loc[2] = ret_long_clust;
                                entry_loc[3] = ret_long_offset;
                            } else {
                                entry_loc[2] = entry_loc[0];
                                entry_loc[3] = entry_loc[1];
                            }
                            entry_loc[1] |= 0x80000000; //indicates that the entry is in the root directory before the clusters
                            return entry_loc;
                        }else{
                            if((ret[11] & FAT_ATTR_SUBDIR) == FAT_ATTR_SUBDIR){ //Subdirectory
                                ret_has_long = 0;
                                cur_clust = ret[0x1A] + (ret[0x1B] << 8) + (ret[0x14] << 16) + (ret[0x15] << 24);
                                loopBreak = 1;
                                break;
                            }else{
                                return 0;
                            }
                        }
                    }
                    ret_has_long = false;
                }
                if(loopBreak) break;
            }
        } else {
            while(cur_clust != 0){
                uint8_t loopBreak = 0;
                
                for(uint32_t j = 0; j < inst->sectorsPerCluster; ++j){
                    //Read current sector
                    uint8_t cur_sec_bytes[512];
                    storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + inst->maxRootDirEntries/16 + (cur_clust - 2)*inst->sectorsPerCluster + j, cur_sec_bytes, 512);
                    
                    for(uint32_t i = 0; i < 512; i += 32){
                        if(cur_sec_bytes[i] == 0x00) return 0; //Free from here on
                        if(cur_sec_bytes[i] == 0xE5) continue; //File was deleted
                        
                        if(cur_sec_bytes[i+11] == 0x0F) { //Part of long filename
                            if(!ret_has_long){
                                ret_long_clust = cur_clust;
                                ret_long_offset = i;
                                ret_has_long = 1;
                            }
                            
                            uint8_t cur_long_fname_temp[] = {cur_sec_bytes[i + 1], cur_sec_bytes[i + 3], cur_sec_bytes[i + 5], cur_sec_bytes[i + 7], cur_sec_bytes[i + 9],
                                        cur_sec_bytes[i + 14], cur_sec_bytes[i + 16], cur_sec_bytes[i + 18], cur_sec_bytes[i + 20], cur_sec_bytes[i + 22], cur_sec_bytes[i + 24],
                                        cur_sec_bytes[i + 28], cur_sec_bytes[i + 30]};
                            for(uint8_t l = 0; l < /*Length of cur_long_fname_temp*/13; ++l){
                                if ((/*if last long file name entry*/(cur_sec_bytes[i] & 0xF0) == 0x40) && (cur_long_fname_temp[l] == 0)) break;
                                long_fname_buffer[((cur_sec_bytes[i] & 0x0F)-1)*13+l] = (char)cur_long_fname_temp[l];
                            }
                            continue;
                        }
                        
                        char cur_filename[FAT_MAX_FNAME_LEN];
                        char cur_ext[FAT_MAX_FNAME_LEN];
                        
                        if(long_fname_buffer[0] == 0){ //if file name is short
                            for(uint8_t l = 0; l < 8; ++l){
                                cur_filename[l] = (char)cur_sec_bytes[i+l];
                            }
                            cur_filename[8] = 0; //Null termination
                            strtrimend(cur_filename);
                            for(uint8_t l = 0; l < 3; ++l){
                                cur_ext[l] = (char)cur_sec_bytes[i+8+l];
                            }
                            cur_ext[3] = 0; //Null termination
                            strtrimend(cur_ext);
                            ret_has_long = 0;
                        }else{ //if file name is long
                            //Split the current file into name and extension
                            cur_file_len = strlen(long_fname_buffer);
                            lastDot = strrchr(long_fname_buffer, '.');
                            if(lastDot == -1) lastDot = cur_file_len;
                            cur_filename[1] = 0; //Null termination if lastDot = 0
                            for(uint32_t l = 0; l < lastDot; ++l){
                                cur_filename[l] = long_fname_buffer[l];
                            }
                            cur_filename[lastDot] = 0; //Null termination
                            for(uint32_t l = (lastDot+1); l < cur_file_len; ++l){
                                cur_ext[l-(lastDot+1)] = long_fname_buffer[l];
                            }
                            cur_ext[cur_file_len-lastDot-1] = 0; //Null termination
                            
                            //Clear long_fname_buffer
                            for(uint8_t l = 0; l < 208; ++l){
                                long_fname_buffer[l] = 0;
                            }
                        }
                        
                        tolower(cur_filename);
                        tolower(cur_file_name);
                        if(strcmp(cur_filename, cur_file_name) == 0){ //if the requested file name matches the file name of the current entry
                            if(*cur_file_ext == 0){ //if cur_file_ext is empty
                                //Copy the current file entry to ret
                                for(uint8_t l = 0; l < 32; ++l){
                                    ret[l] = cur_sec_bytes[i+l];
                                }
                            }else{
                                tolower(cur_ext);
                                tolower(cur_file_ext);
                                if(strcmp(cur_ext, cur_file_ext) == 0){ //if the requested file extension matches the file extension of the current entry
                                    //Copy the current file entry to ret
                                    for(uint8_t l = 0; l < 32; ++l){
                                        ret[l] = cur_sec_bytes[i+l];
                                    }
                                }
                                else{
                                    ret_has_long = 0;
                                    continue;
                                }
                            }
                            
                            if(k == (path_size-1)){ //if is final file
                                uint32_t* entry_loc = malloc(16, 0);
                                entry_loc[0] = cur_clust;
                                entry_loc[1] = j * 512 + i;
                                if(ret_has_long) {
                                    entry_loc[2] = ret_long_clust;
                                    entry_loc[3] = ret_long_offset;
                                } else {
                                    entry_loc[2] = entry_loc[0];
                                    entry_loc[3] = entry_loc[1];
                                }
                                return entry_loc;
                            }else{
                                if((ret[11] & FAT_ATTR_SUBDIR) == FAT_ATTR_SUBDIR){ //Subdirectory
                                    ret_has_long = 0;
                                    cur_clust = ret[0x1A] + (ret[0x1B] << 8) + (ret[0x14] << 16) + (ret[0x15] << 24);
                                    loopBreak = 1;
                                    break;
                                }else{
                                    return 0;
                                }
                            }
                        }
                        ret_has_long = false;
                    }
                    if(loopBreak) break;
                }
                if(loopBreak) break;
                cur_clust = FAT_FATgetCluster(inst, cur_clust);
            }
        }
    }
    return 0;
}

uint8_t FAT_writeFileContents(FAT* inst, char* filepath, uint8_t* contents, uint32_t length){
    uint32_t* entry_loc = FAT_readFileEntryLocation(inst, filepath);
    if(entry_loc == 0) return 0;
    uint8_t isRootDirOfFAT12Or16 = 0;
    if(entry_loc[1] & 0x80000000){
        isRootDirOfFAT12Or16 = 1;
        entry_loc[1] &= 0x7FFFFFFF;
    }
    uint8_t entry_sec_bytes[512];
    if(isRootDirOfFAT12Or16){ // if file is in the root directory of a FAT12 or FAT16 file system
        storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + entry_loc[0], entry_sec_bytes, 512);
    } else {
        storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + inst->maxRootDirEntries/16 + (entry_loc[0] - 2)*inst->sectorsPerCluster + entry_loc[1]/512, entry_sec_bytes, 512);
    }
    uint8_t entry[32];
    memcpy(entry, entry_sec_bytes + (entry_loc[1] % 512), 32);
    
    uint32_t cur_clust = entry[0x1A] + (entry[0x1B] << 8) + (entry[0x14] << 16) + (entry[0x15] << 24);
    uint32_t prev_clust = 0;
    
    if(cur_clust == 0){
        cur_clust = FAT_FATfindFreeCluster(inst);
        entry[0x1A] = cur_clust;
        entry[0x1B] = (cur_clust >> 8);
        entry[0x14] = (cur_clust >> 16);
        entry[0x15] = (cur_clust >> 24);
    }
    entry[0x1C] = length;
    entry[0x1D] = (length >> 8);
    entry[0x1E] = (length >> 16);
    entry[0x1F] = (length >> 24);
    memcpy(entry_sec_bytes + (entry_loc[1] % 512), entry, 32);
    free(entry_loc);
    if(isRootDirOfFAT12Or16){ // if file is in the root directory of a FAT12 or FAT16 file system
        storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT * inst->FATCopies + entry_loc[0], entry_sec_bytes, 512);
    } else {
        storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT * inst->FATCopies + inst->maxRootDirEntries/16 + (entry_loc[0] - 2) * inst->sectorsPerCluster + entry_loc[1]/512, entry_sec_bytes, 512);
    }
    
    for(uint32_t i = 0; i <= (length / 512) / inst->sectorsPerCluster; ++i){
        if(prev_clust != 0){
            //Write FAT
            FAT_FATsetCluster(inst, prev_clust, cur_clust);
        }
        
        for(uint32_t j = 0; j < inst->sectorsPerCluster; ++j){
            uint8_t cur_content_sec[512] = {0};
            memcpy(cur_content_sec, contents + i*512*inst->sectorsPerCluster + j * 512, ((i * 512 * inst->sectorsPerCluster + j * 512 + 512) > length) ? (length % 512) : 512);
            //Write content
            storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT * inst->FATCopies + inst->maxRootDirEntries/16 + (cur_clust - 2) * inst->sectorsPerCluster + j, cur_content_sec, 512);
        }
        prev_clust = cur_clust;
        //If the cluster chain has ended, just find a free cluster
        if(FAT_FATgetClusterDetailed(inst, cur_clust) == 0){
            FAT_FATsetCluster(inst, cur_clust, (inst->fileSys == FAT12) ? 0x0FF8 : ((inst->fileSys == FAT16) ? 0xFFF8 : 0x0FFFFFF8));
            cur_clust = FAT_FATfindFreeCluster(inst);
            FAT_FATsetCluster(inst, prev_clust, 0);
        }
        else if(FAT_FATgetClusterDetailed(inst, cur_clust) >= ( (inst->fileSys == FAT12) ? 0x0FF8 : ((inst->fileSys == FAT16) ? 0xFFF8 : 0x0FFFFFF8) )){
            cur_clust = FAT_FATfindFreeCluster(inst);
        }
        else{
            cur_clust = FAT_FATgetCluster(inst, cur_clust);
        }
    }
    
    FAT_FATsetCluster(inst, prev_clust, (inst->fileSys == FAT12) ? 0x0FF8 : ((inst->fileSys == FAT16) ? 0xFFF8 : 0x0FFFFFF8)); //Last cluster
    
    //Clean up the remaining clusters
    /*while(cur_clust != 0){
        prev_clust = cur_clust;
        cur_clust = FAT_FATgetCluster(inst, cur_clust);
        FAT_FATsetCluster(inst, prev_clust, 0);
    }*/
    FAT_FATclearClusterChain(inst, cur_clust);
    return 1;
}

uint8_t FAT_createDirectory(FAT* inst, char* filepath){
    uint8_t* entry = FAT_createFile(inst, filepath, FAT_ATTR_SUBDIR);
    if(entry == 0) return 0; //Directory could not be created
    
    uint8_t new_dir_entry[32];
    memcpy(new_dir_entry, entry, 32);
    free(entry);
    //Directory created or found
    //Format and split the file path
    char spl_path[FAT_MAX_PATH_FILES][FAT_MAX_FNAME_LEN];
    for(uint32_t k = 0; k < FAT_MAX_PATH_FILES; ++k){
        for(uint32_t l = 0; l < FAT_MAX_FNAME_LEN; ++l){
            spl_path[k][l] = 0;
        }
    }
    char delimiter[] = "/";
    int path_size = 0;
    formatFilePath(filepath);
    size_t tmp_filepath_len = strlen(filepath)+1;
    char tmp_filepath[tmp_filepath_len];
    memset(tmp_filepath, 0, tmp_filepath_len);
    strcpy(tmp_filepath, filepath);
    char* cur_file = strtok(tmp_filepath, delimiter);
    while(cur_file && path_size < FAT_MAX_PATH_FILES){
        if(strlen(cur_file) > FAT_MAX_FNAME_LEN-1) break; //make sure current filename is not to long
        strcpy(spl_path[path_size], cur_file);
        cur_file = strtok(0, delimiter);
        ++path_size;
    }
    
    //Get the file container path
    uint16_t path_container_len = 0;
    for(uint32_t k = 0; k < (path_size-1); ++k){
        for(uint32_t l = 0; l < FAT_MAX_FNAME_LEN; ++l){
            ++path_container_len; //This line is before the break, because there needs to be space for the '/' and the null termination
            if(spl_path[k][l] == 0) break;
        }
    }
    char path_container[path_container_len];
    memset(path_container, 0, path_container_len);
    uint16_t i = 0;
    for(uint32_t k = 0; k < (path_size-1); ++k){
        for(uint32_t l = 0; l < FAT_MAX_FNAME_LEN; ++l){
            if(spl_path[k][l] == 0) break;
            path_container[i] = spl_path[k][l];
            ++i;
        }
        if(path_container_len == (i+1)) break;
        path_container[i] = '/';
        ++i;
    }
    
    uint32_t* entry_loc = FAT_readFileEntryLocation(inst, filepath);
    if(entry_loc == 0) return 0; //Could not find the entry to the new directory
    uint8_t isRootDirOfFAT12Or16 = 0;
    if(entry_loc[1] & 0x80000000){
        isRootDirOfFAT12Or16 = 1;
        entry_loc[1] &= 0x7FFFFFFF;
    }
    uint8_t entry_sec_bytes[512];
    if(isRootDirOfFAT12Or16){
        storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + entry_loc[0], entry_sec_bytes, 512);
    } else {
        storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + inst->maxRootDirEntries/16 + (entry_loc[0] - 2)*inst->sectorsPerCluster + entry_loc[1]/512, entry_sec_bytes, 512);
    }
    
    uint32_t cur_clust = FAT_FATfindFreeCluster(inst);
    new_dir_entry[0x1A] = cur_clust;
    new_dir_entry[0x1B] = (cur_clust >> 8);
    new_dir_entry[0x14] = (cur_clust >> 16);
    new_dir_entry[0x15] = (cur_clust >> 24);
    
    memcpy(entry_sec_bytes + (entry_loc[1] % 512), new_dir_entry, 32);
    if(isRootDirOfFAT12Or16){
        storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT * inst->FATCopies + entry_loc[0], entry_sec_bytes, 512);
    } else {
        storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT * inst->FATCopies + inst->maxRootDirEntries/16 + (entry_loc[0] - 2) * inst->sectorsPerCluster + entry_loc[1]/512, entry_sec_bytes, 512);
    }
    
    uint8_t dir_contents[] = { '.', 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x10, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, (cur_clust >> 16), (cur_clust >> 24), 0x00, 0x00, 0x00, 0x00, cur_clust, (cur_clust >> 8), 0x00, 0x00, 0x00, 0x00,
        '.', '.', 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x10, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, (entry_loc[0] >> 16), (entry_loc[0] >> 24), 0x00, 0x00, 0x00, 0x00, entry_loc[0], (entry_loc[0] >> 8), 0x00, 0x00, 0x00, 0x00};
    free(entry_loc);
    //Write content
    storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT * inst->FATCopies + inst->maxRootDirEntries/16 + (cur_clust - 2) * inst->sectorsPerCluster, dir_contents, 64);
    
    //Last cluster
    FAT_FATsetCluster(inst, cur_clust, (inst->fileSys == FAT12) ? 0x0FF8 : ((inst->fileSys == FAT16) ? 0xFFF8 : 0x0FFFFFF8));
    
    return 1; //Directory contents successfully saved
}

void FAT_deleteFile(FAT* inst, char* filepath){
    //TODO: handle deletion of directory
    uint8_t* entry = FAT_readFileEntry(inst, filepath);
    if(entry == 0) return; //File doesn't exist
    if(entry[11] & FAT_ATTR_SUBDIR){
        file_t file_inst = { {0}, {0}, 0, 0, 0 };
        uint32_t i = 0;
        while(FAT_readFileEntryByIndex(inst, &file_inst, filepath, i)){
            if(strcmp(file_inst.name, ".") && strcmp(file_inst.name, "..")){
                size_t tmp_filepath_len = strlen(filepath)+strlen(file_inst.name)+2;
                char tmp_filepath[tmp_filepath_len];
                memset(tmp_filepath, 0, tmp_filepath_len);
                strcpy(tmp_filepath, filepath);
                strcat(tmp_filepath, "/");
                strcat(tmp_filepath, file_inst.name);
                FAT_deleteFile(inst, tmp_filepath);
            }
            ++i;
        }
    }
    uint32_t* entry_loc = FAT_readFileEntryLocation(inst, filepath);
    //entry_loc[0]    cluster of short name entry
    //entry_loc[1]    offset from start of cluster of short name entry
    //entry_loc[2]    cluster of long name entry (if there is none then same as entry_loc[0])
    //entry_loc[3]    offset from start of cluster of long name entry (if there is none then same as entry_loc[1])
    uint8_t isRootDirOfFAT12Or16 = 0;
    if(entry_loc[1] & 0x80000000){
        isRootDirOfFAT12Or16 = 1;
        entry_loc[1] &= 0x7FFFFFFF;
    }
    
    uint8_t entry_sec_bytes[512];
    
    if(isRootDirOfFAT12Or16){
        if(entry_loc[0] == entry_loc[2]) {
            for(uint32_t j = entry_loc[3]; j <= entry_loc[1]; j += 32){
                storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + entry_loc[0], entry_sec_bytes, 512);
                entry_sec_bytes[j % 512] = 0xE5;
                storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + entry_loc[0], entry_sec_bytes, 512);
            }
        } else {
            //Still in the first of the two clusters
            for(uint32_t j = entry_loc[3]; j <= 512; j += 32){
                storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + entry_loc[2], entry_sec_bytes, 512);
                entry_sec_bytes[j % 512] = 0xE5;
                storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + entry_loc[2], entry_sec_bytes, 512);
            }
            //Already in the second of the two clusters
            for(uint32_t j = 0; j <= entry_loc[1]; j += 32){
                storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + entry_loc[0], entry_sec_bytes, 512);
                entry_sec_bytes[j % 512] = 0xE5;
                storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + entry_loc[0], entry_sec_bytes, 512);
            }
        }
    } else {
        if(entry_loc[0] == entry_loc[2]) {
            //Still in the first of the two clusters
            for(uint32_t j = entry_loc[3]; j <= entry_loc[1]; j += 32){
                storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + inst->maxRootDirEntries/16 + (entry_loc[0] - 2)*inst->sectorsPerCluster + j/512, entry_sec_bytes, 512);
                entry_sec_bytes[j % 512] = 0xE5;
                storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + inst->maxRootDirEntries/16 + (entry_loc[0] - 2)*inst->sectorsPerCluster + j/512, entry_sec_bytes, 512);
            }
        } else {
            //Still in the first of the two clusters
            for(uint32_t j = entry_loc[3]; j <= inst->sectorsPerCluster*512; j += 32){
                storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + inst->maxRootDirEntries/16 + (entry_loc[2] - 2)*inst->sectorsPerCluster + j/512, entry_sec_bytes, 512);
                entry_sec_bytes[j % 512] = 0xE5;
                storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + inst->maxRootDirEntries/16 + (entry_loc[2] - 2)*inst->sectorsPerCluster + j/512, entry_sec_bytes, 512);
            }
            //Already in the second of the two clusters
            for(uint32_t j = 0; j <= entry_loc[1]; j += 32){
                storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + inst->maxRootDirEntries/16 + (entry_loc[0] - 2)*inst->sectorsPerCluster + j/512, entry_sec_bytes, 512);
                entry_sec_bytes[j % 512] = 0xE5;
                storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + inst->maxRootDirEntries/16 + (entry_loc[0] - 2)*inst->sectorsPerCluster + j/512, entry_sec_bytes, 512);
            }
        }
    }
    free(entry_loc);
    
    uint32_t cur_clust = entry[0x1A] + (entry[0x1B] << 8) + (entry[0x14] << 16) + (entry[0x15] << 24);
    free(entry);
    //Clean up the remaining clusters
    FAT_FATclearClusterChain(inst, cur_clust);
    /*listHead_t* clustList = FAT_FATgetClusterChain(inst, cur_clust, 0, FAT_getFilesize(entry));
    uint32_t clustList_index = 1;
    while(cur_clust != 0){
        FAT_FATsetCluster(inst, cur_clust, 0);
        cur_clust = (uint32_t)listShowElement(clustList, ++clustList_index);
    }
    listDeleteAllWithoutData(clustList);*/
    /*uint32_t prev_clust = 0;
    while(cur_clust != 0){
        prev_clust = cur_clust;
        cur_clust = FAT_FATgetCluster(inst, cur_clust);
        FAT_FATsetCluster(inst, prev_clust, 0);
    }*/
}

char* FAT_readVolumeLabel(FAT* inst){
    if(inst->fileSys <= FAT16){
        for(uint8_t j = 0; j < inst->maxRootDirEntries/16; ++j){
            uint8_t cur_sec_bytes[512];
            storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + j, cur_sec_bytes, 512);
            for(uint16_t i = 0; i < 512; i += 32){
                if(cur_sec_bytes[i + 11] == FAT_ATTR_VOLLABEL){
                    //Entry is a volume label
                    static char ret[12];
                    for(uint8_t k = 0; k < 11; ++k){
                        ret[k] = cur_sec_bytes[i+k];
                    }
                    ret[12] = 0;
                    return strtrimend(ret);
                }
            }
        }
    } else {
        uint32_t cur_clust = inst->startClustOfRootDir;
        
        while(cur_clust != 0){
            for(uint8_t j = 0; j < inst->sectorsPerCluster; ++j){
                uint8_t cur_sec_bytes[512];
                storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + (cur_clust - 2)*inst->sectorsPerCluster + j, cur_sec_bytes, 512);
                for(uint16_t i = 0; i < 512; i += 32){
                    if(cur_sec_bytes[i + 11] == FAT_ATTR_VOLLABEL){
                        //Entry is a volume label
                        static char ret[12];
                        for(uint8_t k = 0; k < 11; ++k){
                            ret[k] = cur_sec_bytes[i+k];
                        }
                        ret[12] = 0;
                        return strtrimend(ret);
                    }
                }
            }
            cur_clust = FAT_FATgetCluster(inst, cur_clust);
        }
    }
    return 0;
}

uint8_t* FAT_readFileEntry(FAT* inst, char* filepath){
    uint8_t* ret = malloc(32, 0);
    //Format and split the file path
    char spl_path[FAT_MAX_PATH_FILES][FAT_MAX_FNAME_LEN];
    for(uint32_t k = 0; k < FAT_MAX_PATH_FILES; ++k){
        for(uint32_t l = 0; l < FAT_MAX_FNAME_LEN; ++l){
            spl_path[k][l] = 0;
        }
    }
    char delimiter[] = "/";
    int path_size = 0;
    formatFilePath(filepath);
    size_t tmp_filepath_len = strlen(filepath)+1;
    char tmp_filepath[tmp_filepath_len];
    memset(tmp_filepath, 0, tmp_filepath_len);
    strcpy(tmp_filepath, filepath);
    char* cur_file = strtok(tmp_filepath, delimiter);
    while(cur_file != 0 && path_size < FAT_MAX_PATH_FILES){
        if(strlen(cur_file) > FAT_MAX_FNAME_LEN-1) break; //make sure current filename is not to long
        strcpy(spl_path[path_size], cur_file);
        cur_file = strtok(0, delimiter);
        ++path_size;
    }
    
    uint32_t cur_clust = inst->startClustOfRootDir;
    
    for(uint32_t k = 0; k < path_size; ++k){
        //Split the current file into name and extension
        int cur_file_len = strlen(spl_path[k]);
        int32_t lastDot = strrchr(spl_path[k], '.');
        if(lastDot == -1) lastDot = cur_file_len;
        char cur_file_name[lastDot+1];
        for(uint32_t j = 0; j < lastDot; ++j){
            cur_file_name[j] = spl_path[k][j];
        }
        cur_file_name[lastDot] = 0; //Null termination
        char cur_file_ext[cur_file_len-lastDot];
        for(uint32_t j = (lastDot+1); j < cur_file_len; ++j){
            cur_file_ext[j-(lastDot+1)] = spl_path[k][j];
        }
        cur_file_ext[MAX(cur_file_len-lastDot-1, 0)] = 0; //Null termination
        
        char long_fname_buffer[208];
        //Clear long_fname_buffer
        for(uint8_t l = 0; l < 208; ++l){
            long_fname_buffer[l] = 0;
        }
        
        
        if((inst->fileSys <= FAT16) && k == 0){ //root directory of FAT12 and FAT16
            
            uint8_t loopBreak = 0;
            for(uint8_t j = 0; j < inst->maxRootDirEntries/16; ++j){
                //Read current sector
                uint8_t cur_sec_bytes[512];
                storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + j, cur_sec_bytes, 512);
                
                for(uint32_t i = 0; i < 512; i += 32){
                    if(cur_sec_bytes[i] == 0x00){
                        free(ret);
                        return 0; //Free from here on
                    }
                    if(cur_sec_bytes[i] == 0xE5) continue; //File was deleted
                    
                    if(cur_sec_bytes[i+11] == 0x0F) { //Part of long filename
                        uint8_t cur_long_fname_temp[] = {cur_sec_bytes[i + 1], cur_sec_bytes[i + 3], cur_sec_bytes[i + 5], cur_sec_bytes[i + 7], cur_sec_bytes[i + 9],
                                    cur_sec_bytes[i + 14], cur_sec_bytes[i + 16], cur_sec_bytes[i + 18], cur_sec_bytes[i + 20], cur_sec_bytes[i + 22], cur_sec_bytes[i + 24],
                                    cur_sec_bytes[i + 28], cur_sec_bytes[i + 30]};
                        for(uint8_t l = 0; l < /*Length of cur_long_fname_temp*/13; ++l){
                            if ((/*if last long file name entry*/(cur_sec_bytes[i] & 0xF0) == 0x40) && (cur_long_fname_temp[l] == 0)) break;
                            long_fname_buffer[((cur_sec_bytes[i] & 0x0F)-1)*13+l] = (char)cur_long_fname_temp[l];
                        }
                        continue;
                    }
                    
                    char cur_filename[FAT_MAX_FNAME_LEN];
                    char cur_ext[FAT_MAX_FNAME_LEN];
                    
                    if(long_fname_buffer[0] == 0){ //if file name is short
                        for(uint8_t l = 0; l < 8; ++l){
                            cur_filename[l] = (char)cur_sec_bytes[i+l];
                        }
                        cur_filename[8] = 0; //Null termination
                        strtrimend(cur_filename);
                        for(uint8_t l = 0; l < 3; ++l){
                            cur_ext[l] = (char)cur_sec_bytes[i+8+l];
                        }
                        cur_ext[3] = 0; //Null termination
                        strtrimend(cur_ext);
                    }else{ //if file name is long
                        //Split the current file into name and extension
                        cur_file_len = strlen(long_fname_buffer);
                        lastDot = strrchr(long_fname_buffer, '.');
                        if(lastDot == -1) lastDot = cur_file_len;
                        cur_filename[1] = 0; //Null termination if lastDot = 0
                        for(uint32_t l = 0; l < lastDot; ++l){
                            cur_filename[l] = long_fname_buffer[l];
                        }
                        cur_filename[lastDot] = 0; //Null termination
                        for(uint32_t l = (lastDot+1); l < cur_file_len; ++l){
                            cur_ext[l-(lastDot+1)] = long_fname_buffer[l];
                        }
                        cur_ext[cur_file_len-lastDot-1] = 0; //Null termination
                        
                        //Clear long_fname_buffer
                        for(uint8_t l = 0; l < 208; ++l){
                            long_fname_buffer[l] = 0;
                        }
                    }
                    
                    tolower(cur_filename);
                    tolower(cur_file_name);
                    if(strcmp(cur_filename, cur_file_name) == 0){ //if the requested file name matches the file name of the current entry
                        if(*cur_file_ext == 0){ //if cur_file_ext is empty
                            //Copy the current file entry to ret
                            for(uint8_t l = 0; l < 32; ++l){
                                ret[l] = cur_sec_bytes[i+l];
                            }
                        }else{
                            tolower(cur_ext);
                            tolower(cur_file_ext);
                            if(strcmp(cur_ext, cur_file_ext) == 0){ //if the requested file extension matches the file extension of the current entry
                                //Copy the current file entry to ret
                                for(uint8_t l = 0; l < 32; ++l){
                                    ret[l] = cur_sec_bytes[i+l];
                                }
                            }
                            else continue;
                        }
                        
                        if(k == (path_size-1)){ //if is final file
                            return ret;
                        }else{
                            if((ret[11] & FAT_ATTR_SUBDIR) == FAT_ATTR_SUBDIR){ //Subdirectory
                                cur_clust = ret[0x1A] + (ret[0x1B] << 8) + (ret[0x14] << 16) + (ret[0x15] << 24);
                                loopBreak = 1;
                                break;
                            }else{
                                free(ret);
                                return 0;
                            }
                        }
                        
                    }
                }
                if(loopBreak) break;
            }
            
        } else {
            while(cur_clust != 0){
                uint8_t loopBreak = 0;
                
                for(uint32_t j = 0; j < inst->sectorsPerCluster; ++j){
                    //Read current sector
                    uint8_t cur_sec_bytes[512];
                    storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + inst->maxRootDirEntries/16 + (cur_clust - 2)*inst->sectorsPerCluster + j, cur_sec_bytes, 512);
                    
                    for(uint32_t i = 0; i < 512; i += 32){
                        if(cur_sec_bytes[i] == 0x00){
                            free(ret);
                            return 0; //Free from here on
                        }
                        if(cur_sec_bytes[i] == 0xE5) continue; //File was deleted
                        
                        if(cur_sec_bytes[i+11] == 0x0F) { //Part of long filename
                            uint8_t cur_long_fname_temp[] = {cur_sec_bytes[i + 1], cur_sec_bytes[i + 3], cur_sec_bytes[i + 5], cur_sec_bytes[i + 7], cur_sec_bytes[i + 9],
                                        cur_sec_bytes[i + 14], cur_sec_bytes[i + 16], cur_sec_bytes[i + 18], cur_sec_bytes[i + 20], cur_sec_bytes[i + 22], cur_sec_bytes[i + 24],
                                        cur_sec_bytes[i + 28], cur_sec_bytes[i + 30]};
                            for(uint8_t l = 0; l < /*Length of cur_long_fname_temp*/13; ++l){
                                if ((/*if last long file name entry*/(cur_sec_bytes[i] & 0xF0) == 0x40) && (cur_long_fname_temp[l] == 0)) break;
                                long_fname_buffer[((cur_sec_bytes[i] & 0x0F)-1)*13+l] = (char)cur_long_fname_temp[l];
                            }
                            continue;
                        }
                        
                        char cur_filename[FAT_MAX_FNAME_LEN];
                        char cur_ext[FAT_MAX_FNAME_LEN];
                        
                        if(long_fname_buffer[0] == 0){ //if file name is short
                            for(uint8_t l = 0; l < 8; ++l){
                                cur_filename[l] = (char)cur_sec_bytes[i+l];
                            }
                            cur_filename[8] = 0; //Null termination
                            strtrimend(cur_filename);
                            for(uint8_t l = 0; l < 3; ++l){
                                cur_ext[l] = (char)cur_sec_bytes[i+8+l];
                            }
                            cur_ext[3] = 0; //Null termination
                            strtrimend(cur_ext);
                        }else{ //if file name is long
                            //Split the current file into name and extension
                            cur_file_len = strlen(long_fname_buffer);
                            lastDot = strrchr(long_fname_buffer, '.');
                            if(lastDot == -1) lastDot = cur_file_len;
                            cur_filename[1] = 0; //Null termination if lastDot = 0
                            for(uint32_t l = 0; l < lastDot; ++l){
                                cur_filename[l] = long_fname_buffer[l];
                            }
                            cur_filename[lastDot] = 0; //Null termination
                            for(uint32_t l = (lastDot+1); l < cur_file_len; ++l){
                                cur_ext[l-(lastDot+1)] = long_fname_buffer[l];
                            }
                            cur_ext[cur_file_len-lastDot-1] = 0; //Null termination
                            
                            //Clear long_fname_buffer
                            for(uint8_t l = 0; l < 208; ++l){
                                long_fname_buffer[l] = 0;
                            }
                        }
                        
                        tolower(cur_filename);
                        tolower(cur_file_name);
                        if(strcmp(cur_filename, cur_file_name) == 0){ //if the requested file name matches the file name of the current entry
                            if(*cur_file_ext == 0){ //if cur_file_ext is empty
                                //Copy the current file entry to ret
                                for(uint8_t l = 0; l < 32; ++l){
                                    ret[l] = cur_sec_bytes[i+l];
                                }
                            }else{
                                tolower(cur_ext);
                                tolower(cur_file_ext);
                                if(strcmp(cur_ext, cur_file_ext) == 0){ //if the requested file extension matches the file extension of the current entry
                                    //Copy the current file entry to ret
                                    for(uint8_t l = 0; l < 32; ++l){
                                        ret[l] = cur_sec_bytes[i+l];
                                    }
                                }
                                else continue;
                            }
                            
                            if(k == (path_size-1)){ //if is final file
                                return ret;
                            }else{
                                if((ret[11] & FAT_ATTR_SUBDIR) == FAT_ATTR_SUBDIR){ //Subdirectory
                                    cur_clust = ret[0x1A] + (ret[0x1B] << 8) + (ret[0x14] << 16) + (ret[0x15] << 24);
                                    loopBreak = 1;
                                    break;
                                }else{
                                    free(ret);
                                    return 0;
                                }
                            }
                            
                        }
                    }
                    if(loopBreak) break;
                }
                if(loopBreak) break;
                cur_clust = FAT_FATgetCluster(inst, cur_clust);
            }
        }
    }
    free(ret);
    return 0;
}

uint32_t FAT_getFilesize(uint8_t* entry) {
    if (entry == 0) return 0;
    return entry[0x1C] + (uint32_t)(entry[0x1D] << 8) + (uint32_t)(entry[0x1E] << 16) + (uint32_t)(entry[0x1F] << 24);
}

uint8_t FAT_readFileByte(FAT* inst, uint8_t* entry) {
    static uint32_t cur_clust;
    static uint32_t j;
    static uint16_t i;
    static uint8_t* cur_sec_bytes;
    if(entry != 0){
        cur_clust = entry[0x1A] + (entry[0x1B] << 8) + (entry[0x14] << 16) + (entry[0x15] << 24);
        j = 0;
        i = 0;
        storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + inst->maxRootDirEntries/16 + (cur_clust - 2)*inst->sectorsPerCluster + j, cur_sec_bytes, 512);
    }
    
    if(cur_clust != 0){
        if(i < 512) {
            return cur_sec_bytes[i++];
        }
        ++j;
        i = 1;
        if(j < inst->sectorsPerCluster){
            storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + inst->maxRootDirEntries/16 + (cur_clust - 2)*inst->sectorsPerCluster + j, cur_sec_bytes, 512);
            return cur_sec_bytes[0];
        }
        
        cur_clust = FAT_FATgetCluster(inst, cur_clust);
        j = 0;
        i = 1;
        if(cur_clust != 0){
            storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + inst->maxRootDirEntries/16 + (cur_clust - 2)*inst->sectorsPerCluster + j, cur_sec_bytes, 512);
            return cur_sec_bytes[0];
        }
    }
    return 0; //No more data (file end)
}

void FAT_readFileContents(FAT* inst, uint8_t* entry, uint8_t* buffer, uint32_t start, uint32_t len){
    if(len == 0) return;
    uint32_t cur_clust = entry[0x1A] + (entry[0x1B] << 8) + (entry[0x14] << 16) + (entry[0x15] << 24);
    uint32_t* clustArr = FAT_FATgetClusterChain(inst, cur_clust, start, len);
    
//    uint8_t cur_sec_bytes[512*inst->sectorsPerCluster];
//    uint32_t pos = (start/(inst->sectorsPerCluster*512))*(inst->sectorsPerCluster*512);
//    uint32_t clustArr_index = 1;
//    if(clustArr[0] == 0) cur_clust = 0;
//    else cur_clust = clustArr[clustArr_index];
//    //uint32_t clustList_index = 1;
//    //cur_clust = (uint32_t)listShowElement(clustList, clustList_index);
//    
//    while(cur_clust != 0){
//        //uint32_t next_cluster = (uint32_t)listShowElement(clustList, clustList_index+1);
//        uint32_t next_cluster = clustArr[clustArr_index+1];
//        
//        if(( pos >= start )
//        && ( (pos+2*512*inst->sectorsPerCluster-1) <= (start+len) )
//        && (next_cluster == (cur_clust+1)) ){ // if the next cluster comes directly after the current one
//            storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + inst->maxRootDirEntries/16 + (cur_clust - 2)*inst->sectorsPerCluster, buffer+pos-start, 2*512*inst->sectorsPerCluster);
//            pos += 2*512*inst->sectorsPerCluster;
//            //++clustList_index;
//            ++clustArr_index;
//            //next_cluster = (uint32_t)listShowElement(clustList, clustList_index+1);
//            next_cluster = clustArr[clustArr_index+1];
//        }
//        else if(( pos >= start )
//        && ( (pos+512*inst->sectorsPerCluster-1) <= (start+len) )){
//            storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + inst->maxRootDirEntries/16 + (cur_clust - 2)*inst->sectorsPerCluster, buffer+pos-start, 512*inst->sectorsPerCluster);
//            pos += 512*inst->sectorsPerCluster;
//        }
//        else {
//            storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + inst->maxRootDirEntries/16 + (cur_clust - 2)*inst->sectorsPerCluster, cur_sec_bytes, 512*inst->sectorsPerCluster);
//            for(uint16_t i = 0; i < 512*inst->sectorsPerCluster; ++i){
//                if((pos >= start) && (pos < (start+len))){
//                    buffer[pos-start] = cur_sec_bytes[i];
//                }
//                if(pos >= (start+len)){
//                    free(clustArr);
//                    //listDeleteAllWithoutData(clustList);
//                    return;
//                }
//                ++pos;
//            }
//        }
//        
//        /*for(uint32_t j = 0; j < inst->sectorsPerCluster; ++j){
//            storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + inst->maxRootDirEntries/16 + (cur_clust - 2)*inst->sectorsPerCluster + j, cur_sec_bytes, 512);
//            for(uint16_t i = 0; i < 512; ++i){
//                if((pos >= start) && (pos < (start+len))){
//                    buffer[pos-start] = cur_sec_bytes[i];
//                }
//                if(pos >= (start+len)){
//                    listDeleteAllWithoutData(clustList);
//                    return;
//                }
//                ++pos;
//            }
//        }*/
//        //++clustList_index;
//        ++clustArr_index;
//        cur_clust = next_cluster;
//    }
    
    uint8_t cur_sec_bytes[512*inst->sectorsPerCluster];
    uint32_t pos = (start/(inst->sectorsPerCluster*512))*(inst->sectorsPerCluster*512);
    if(clustArr[0] == 0) cur_clust = 0;
    else cur_clust = clustArr[1];
    
    for(uint32_t clustArr_index = 1; clustArr_index <= clustArr[0]; ++clustArr_index){
        uint32_t next_cluster = clustArr[clustArr_index+1];
        
        if(( pos >= start )
        && ( (pos+2*512*inst->sectorsPerCluster-1) <= (start+len) )
        && (next_cluster == (cur_clust+1)) ){ // if the next cluster comes directly after the current one
            storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + inst->maxRootDirEntries/16 + (cur_clust - 2)*inst->sectorsPerCluster, buffer+pos-start, 2*512*inst->sectorsPerCluster);
            pos += 2*512*inst->sectorsPerCluster;
            ++clustArr_index;
            next_cluster = clustArr[clustArr_index+1];
        }
        else if(( pos >= start )
        && ( (pos+512*inst->sectorsPerCluster-1) <= (start+len) )){
            storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + inst->maxRootDirEntries/16 + (cur_clust - 2)*inst->sectorsPerCluster, buffer+pos-start, 512*inst->sectorsPerCluster);
            pos += 512*inst->sectorsPerCluster;
        }
        else {
            storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + inst->maxRootDirEntries/16 + (cur_clust - 2)*inst->sectorsPerCluster, cur_sec_bytes, 512*inst->sectorsPerCluster);
            for(uint16_t i = 0; i < 512*inst->sectorsPerCluster; ++i){
                if((pos >= start) && (pos < (start+len))){
                    buffer[pos-start] = cur_sec_bytes[i];
                }
                if(pos >= (start+len)){
                    free(clustArr);
                    return;
                }
                ++pos;
            }
        }
        
        cur_clust = next_cluster;
    }
    
    free(clustArr);
    //listDeleteAllWithoutData(clustList);
}

char* FAT_longNameToShortName(FAT* inst, char* filepath){
    //Format and split the file path
    char spl_path[FAT_MAX_PATH_FILES][FAT_MAX_FNAME_LEN];
    for(uint32_t k = 0; k < FAT_MAX_PATH_FILES; ++k){
        for(uint32_t l = 0; l < FAT_MAX_FNAME_LEN; ++l){
            spl_path[k][l] = 0;
        }
    }
    char delimiter[] = "/";
    int path_size = 0;
    formatFilePath(filepath);
    size_t tmp_filepath_len = strlen(filepath)+1;
    char tmp_filepath[tmp_filepath_len];
    memset(tmp_filepath, 0, tmp_filepath_len);
    strcpy(tmp_filepath, filepath);
    char* cur_file = strtok(tmp_filepath, delimiter);
    while(cur_file != 0 && path_size < FAT_MAX_PATH_FILES){
        if(strlen(cur_file) > FAT_MAX_FNAME_LEN-1) break; //make sure current filename is not to long
        strcpy(spl_path[path_size], cur_file);
        cur_file = strtok(0, delimiter);
        ++path_size;
    }
    
    //Get the file container path
    uint16_t path_container_len = 0;
    for(uint32_t k = 0; k < (path_size-1); ++k){
        for(uint32_t l = 0; l < FAT_MAX_FNAME_LEN; ++l){
            ++path_container_len; //This line is before the break, because there needs to be space for the '/' and the null termination
            if(spl_path[k][l] == 0) break;
        }
    }
    char path_container[path_container_len];
    memset(path_container, 0, path_container_len);
    uint16_t i = 0;
    for(uint32_t k = 0; k < (path_size-1); ++k){
        for(uint32_t l = 0; l < FAT_MAX_FNAME_LEN; ++l){
            if(spl_path[k][l] == 0) break;
            path_container[i] = spl_path[k][l];
            ++i;
        }
        if(path_container_len == (i+1)) break;
        path_container[i] = '/';
        ++i;
    }
    
    uint8_t* entry;
    
    //Get the start cluster of the container
    uint32_t start_clust;
    uint32_t cur_clust;
    if(path_container_len == 0){
        start_clust = inst->startClustOfRootDir; //file is in the root directory
    }else{
        entry = FAT_readFileEntry(inst, path_container);
        if(entry == 0) return 0;
        start_clust = entry[0x1A] + (entry[0x1B] << 8) + (entry[0x14] << 16) + (entry[0x15] << 24);
        free(entry);
    }
    cur_clust = start_clust;
    
    ///Get the short file name without tilda
    char* file_str = spl_path[path_size - 1];
    toupper(file_str);
    
    //Split the new file into name and extension
    size_t file_str_len = strlen(file_str);
    int32_t lastDot = strrchr(file_str, '.');
    if(lastDot == -1) lastDot = file_str_len;
    char filename[lastDot+1];
    for(uint32_t j = 0; j < lastDot; ++j){
        filename[j] = file_str[j];
    }
    filename[lastDot] = 0; //Null termination
    char fileext[file_str_len-lastDot];
    for(uint32_t j = (lastDot+1); j < file_str_len; ++j){
        fileext[j-(lastDot+1)] = file_str[j];
    }
    fileext[file_str_len-lastDot] = 0; //Null termination
    
    static char short_name[11];
    memset(short_name, 0x20, 11);
    for(uint8_t k = 0; k < MIN(strlen(filename), 8); ++k){
        short_name[k] = filename[k];
    }
    for(uint8_t k = 0; k < MIN(file_str_len-lastDot, 3); ++k){
        short_name[8+k] = fileext[k];
    }
    
    uint16_t aftild = 1;
    uint8_t loopBreak = 0;
    
    short_name[6] = '~';
    short_name[7] = '1';
    
    if((inst->fileSys <= FAT16) && (path_container_len == 0)){ //root directory of FAT12 and FAT16
        for(uint8_t j = 0; j < inst->maxRootDirEntries/16; ++j){
            //Read current sector
            uint8_t cur_sec_bytes[512];
            storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + j, cur_sec_bytes, 512);
            for(i = 0; i < 512; i += 32){
                if(cur_sec_bytes[i] == 0){
                    loopBreak = 1;
                    break;
                }
                if((cur_sec_bytes[i+11] == 0x0F) || (cur_sec_bytes[i] == 0xE5)) continue;
                char cur_filename[11];
                for(uint8_t k = 0; k < 11; ++k){
                    cur_filename[k] = cur_sec_bytes[i+k];
                }
                if(strncmp(cur_filename, short_name, 11) == 0){
                    //File entry with this name already exists, let's try a different name
                    ++aftild;
                    if (aftild < 10) {
                        short_name[6] = '~';
                        short_name[7] = '0' + aftild;
                    } else if (aftild < 100) {
                        short_name[5] = '~';
                        short_name[6] = '0' + aftild / 10;
                        short_name[7] = '0' + (aftild % 10);
                    } else if (aftild < 1000) {
                        short_name[4] = '~';
                        short_name[5] = '0' + aftild / 100;
                        short_name[6] = '0' + (aftild % 100) / 10;
                        short_name[7] = '0' + (aftild % 10);
                    }
                    j = 0;
                    break;
                }
            }
            if (loopBreak) break;
        }
    } else {
        while(cur_clust != 0){
            uint8_t loopRepeat = 0;
            for(uint32_t j = 0; j < inst->sectorsPerCluster; ++j){
                //Read current sector
                uint8_t cur_sec_bytes[512];
                storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + inst->maxRootDirEntries/16 + (cur_clust - 2)*inst->sectorsPerCluster + j, cur_sec_bytes, 512);
                for(i = 0; i < 512; i += 32){
                    if(cur_sec_bytes[i] == 0){
                        loopBreak = 1;
                        break;
                    }
                    if((cur_sec_bytes[i+11] == 0x0F) || (cur_sec_bytes[i] == 0xE5)) continue;
                    char cur_filename[11];
                    for(uint8_t k = 0; k < 11; ++k){
                        cur_filename[k] = cur_sec_bytes[i+k];
                    }
                    if(strncmp(cur_filename, short_name, 11) == 0){
                        //File entry with this name already exists, let's try a different name
                        ++aftild;
                        if (aftild < 10) {
                            short_name[6] = '~';
                            short_name[7] = '0' + aftild;
                        } else if (aftild < 100) {
                            short_name[5] = '~';
                            short_name[6] = '0' + aftild / 10;
                            short_name[7] = '0' + (aftild % 10);
                        } else if (aftild < 1000) {
                            short_name[4] = '~';
                            short_name[5] = '0' + aftild / 100;
                            short_name[6] = '0' + (aftild % 100) / 10;
                            short_name[7] = '0' + (aftild % 10);
                        }
                        loopRepeat = 1;
                        break;
                    }
                }
                if (loopBreak || loopRepeat) break;
            }
            if(loopRepeat){
                cur_clust = start_clust;
                continue;
            }
            if(loopBreak) break;
            cur_clust = FAT_FATgetCluster(inst, cur_clust);
        }
    }
    
    return short_name; //return the short file name
}

uint8_t* FAT_createFile(FAT* inst, char* filepath, uint8_t attribute){
    uint8_t* entry = FAT_readFileEntry(inst, filepath);
    if(entry != 0) return entry; //File already exists
    
    //Format and split the file path
    char spl_path[FAT_MAX_PATH_FILES][FAT_MAX_FNAME_LEN];
    for(uint32_t k = 0; k < FAT_MAX_PATH_FILES; ++k){
        for(uint32_t l = 0; l < FAT_MAX_FNAME_LEN; ++l){
            spl_path[k][l] = 0;
        }
    }
    char delimiter[] = "/";
    int path_size = 0;
    formatFilePath(filepath);
    size_t tmp_filepath_len = strlen(filepath)+1;
    char tmp_filepath[tmp_filepath_len];
    memset(tmp_filepath, 0, tmp_filepath_len);
    strcpy(tmp_filepath, filepath);
    char* cur_file = strtok(tmp_filepath, delimiter);
    while(cur_file != 0 && path_size < FAT_MAX_PATH_FILES){
        if(strlen(cur_file) > FAT_MAX_FNAME_LEN-1) break; //make sure current filename is not to long
        strcpy(spl_path[path_size], cur_file);
        cur_file = strtok(0, delimiter);
        ++path_size;
    }
    
    //Get the file container path
    char spl_path_container[path_size-1][FAT_MAX_FNAME_LEN];
    for(uint32_t k = 0; k < (path_size-1); ++k){
        for(uint32_t l = 0; l < FAT_MAX_FNAME_LEN; ++l){
            spl_path_container[k][l] = spl_path[k][l];
        }
    }
    
    uint32_t cur_clust;
    uint32_t prev_clust = 0;
    
    if(path_size <= 1){
        //Container is the root directory
        cur_clust = inst->startClustOfRootDir; // cur_clust is going to be 0 if fileSys is FAT12 or FAT16
    }else{
        //Join spl_path_container together to path_container
        uint16_t path_container_len = 0;
        for(uint32_t k = 0; k < (path_size-1); ++k){
            for(uint32_t l = 0; l < FAT_MAX_FNAME_LEN; ++l){
                ++path_container_len; //This line is before the break, because there needs to be space for the '/' and the null termination
                if(spl_path_container[k][l] == 0) break;
            }
        }
        char path_container[path_container_len];
        memset(path_container, 0, path_container_len);
        uint16_t i = 0;
        for(uint32_t k = 0; k < (path_size-1); ++k){
            for(uint32_t l = 0; l < FAT_MAX_FNAME_LEN; ++l){
                if(spl_path_container[k][l] == 0) break;
                path_container[i] = spl_path_container[k][l];
                ++i;
            }
            if(path_container_len == (i+1)) break;
            path_container[i] = '/';
            ++i;
        }
        
        //Read file entry of the container
        entry = FAT_readFileEntry(inst, path_container);
        if(entry == 0) return 0;
        cur_clust = entry[0x1A] + (entry[0x1B] << 8) + (entry[0x14] << 16) + (entry[0x15] << 24);
        free(entry);
    }
    
    
    char* new_file_str = spl_path[path_size - 1];
    
    //Split the new file into name and extension
    int32_t cur_file_len = strlen(new_file_str);
    int32_t lastDot = strrchr(new_file_str, '.');
    if(lastDot == -1) lastDot = cur_file_len;
    char new_filename[lastDot+1];
    for(uint32_t j = 0; j < lastDot; ++j){
        new_filename[j] = new_file_str[j];
    }
    new_filename[lastDot] = 0; //Null termination
    char new_fileext[cur_file_len-lastDot]; //TODO: I think there has be a +1
    for(uint32_t j = (lastDot+1); j < cur_file_len; ++j){
        new_fileext[j-(lastDot+1)] = new_file_str[j];
    }
    new_fileext[MAX(cur_file_len-lastDot-1, 0)] = 0; //Null termination
    
    uint8_t is_long_fname = 0;
    char* new_filenameext;
    uint8_t long_fname_entry_count = 0;
    uint8_t long_fname_entry_index = 0;
    uint8_t* long_fname_entry_buffer = 0;
    entry = malloc(32, 0);//entry = (uint8_t [32]){0};
    if((strlen(new_filename) > 8) || (strlen(new_fileext) > 3)){ //if long filename
        is_long_fname = 1;
        if(strlen(new_fileext) == 0){
            new_filenameext = new_filename;
        }else{
            new_filenameext = new_file_str;
        }
        for(uint8_t l = 0; l < (strlen(new_filenameext) + 13); l += 13){
            ++long_fname_entry_count;
        }
        
        long_fname_entry_buffer = malloc(long_fname_entry_count*32, 0);
        
        char* new_fname_short = FAT_longNameToShortName(inst, filepath); //get the short name
        
        for(uint8_t l = 0; l < strlen(new_filenameext); l += 13){
            entry[0] = ((l+13) < strlen(new_filenameext)) ? ((uint8_t)(l / 13 + 1)) : ((uint8_t)(0x41 + l / 13));
            uint8_t i;
            for(i = 1; i < 11; ++i){
                entry[i] = 0xFF;
            }
            entry[11] = 0x0F; //attribute
            entry[12] = 0x00;
            entry[13] = BSDChecksum(new_fname_short);
            for(i = 14; i < 26; ++i){
                entry[i] = 0xFF;
            }
            entry[26] = 0x00;
            entry[27] = 0x00;
            entry[28] = 0xFF;
            entry[29] = 0xFF;
            entry[30] = 0xFF;
            entry[31] = 0xFF;
            
            for(uint8_t k = 0; k < MIN(strlen(new_filenameext) + 1 - l, 13); ++k) {
                if (k < 5) {
                    entry[1 + k * 2] = new_filenameext[l + k];
                    entry[1 + k * 2 + 1] = 0x00;
                } else if (k < 11) {
                    entry[4 + k * 2] = new_filenameext[l + k];
                    entry[4 + k * 2 + 1] = 0x00;
                } else if (k < 13) {
                    entry[6 + k * 2] = new_filenameext[l + k];
                    entry[6 + k * 2 + 1] = 0x00;
                }
            }
            
            for(i = 0; i < 32; ++i){
                long_fname_entry_buffer[(long_fname_entry_count - 2) * 32 - (l /13)*32 + i] = entry[i];
            }
        }
        
        //Short file entry
        for(uint8_t k = 0; k < 11; ++k){
            entry[k] = new_fname_short[k];
        }
        entry[11] = attribute;
        entry[12] = 0x00;
        entry[13] = 0x00; //TODO: Time and date values
        entry[14] = 0x00;
        entry[15] = 0x00;
        entry[0x10] = 0x00;
        entry[0x11] = 0x00;
        entry[0x12] = 0x00;
        entry[0x13] = 0x00;
        entry[0x16] = 0x00;
        entry[0x17] = 0x00;
        entry[0x18] = 0x00;
        entry[0x19] = 0x00;
        entry[0x1C] = 0x00; //Filesize
        entry[0x1D] = 0x00;
        entry[0x1E] = 0x00;
        entry[0x1F] = 0x00;
        entry[0x14] = 0x00; //First cluster
        entry[0x15] = 0x00;
        entry[0x1A] = 0x00;
        entry[0x1B] = 0x00;
        
        for(uint8_t i = 0; i < 32; ++i){
            long_fname_entry_buffer[long_fname_entry_count*32-32 + i] = entry[i];
        }
    }
    
    
    uint8_t cur_sec_bytes[512];
    
    if((inst->fileSys <= FAT16) && (path_size == 1)){ //root directory of FAT12 and FAT16
        for(uint8_t j = 0; j < inst->maxRootDirEntries/16; ++j) {
            storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + j, cur_sec_bytes, 512);
            for(uint16_t i = 0; i < 512; i += 32) {
                
                if ((cur_sec_bytes[i] == 0x00) || (cur_sec_bytes[i] == 0xE5)){
                    if (is_long_fname) {
                        for(uint16_t l = i; l < 512; l += 32){
                            for(uint8_t k = 0; k < 32; ++k){
                                cur_sec_bytes[l+k] = long_fname_entry_buffer[long_fname_entry_index * 32 + k];
                            }
                            ++long_fname_entry_index;
                            if (long_fname_entry_index == long_fname_entry_count) break; //Everything has been written
                        }
                        storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT * inst->FATCopies + j, cur_sec_bytes, 512);
                        
                        if (long_fname_entry_index == long_fname_entry_count){
                            //Everything has been written
                            free(long_fname_entry_buffer);
                            return entry;
                        }
                    } else {
                        uint16_t k;
                        for(k = 0; k < MIN(/*new_filename.Length*/lastDot, 8); ++k){
                            entry[k] = new_filename[k];
                        }
                        //Fill in the spaces for the file name
                        for(; k < 8; ++k){
                            entry[k] = 0x20;
                        }
                        for(k = 0; k < MIN(/*new_fileext.Length*/cur_file_len-lastDot, 3); ++k){
                            entry[8 + k] = new_fileext[k];
                        }
                        //Fill in the spaces for the file extension
                        for(; k < 3; ++k){
                            entry[8 + k] = 0x20;
                        }
                        
                        entry[11] = attribute;
                        entry[12] = 0x00;
                        entry[13] = 0x00; //TODO: Time and date values
                        entry[14] = 0x00;
                        entry[15] = 0x00;
                        entry[0x10] = 0x00;
                        entry[0x11] = 0x00;
                        entry[0x12] = 0x00;
                        entry[0x13] = 0x00;
                        entry[0x16] = 0x00;
                        entry[0x17] = 0x00;
                        entry[0x18] = 0x00;
                        entry[0x19] = 0x00;
                        entry[0x1C] = 0x00; //Filesize
                        entry[0x1D] = 0x00;
                        entry[0x1E] = 0x00;
                        entry[0x1F] = 0x00;
                        entry[0x14] = 0x00; //First cluster
                        entry[0x15] = 0x00;
                        entry[0x1A] = 0x00;
                        entry[0x1B] = 0x00;
                        
                        for(k = 0; k < 32; ++k){
                            cur_sec_bytes[i+k] = entry[k];
                        }
                        storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT * inst->FATCopies + j, cur_sec_bytes, 512);
                        return entry;
                    }
                }
            }
        }
    } else {
        
        while(cur_clust != 0) {
            for(uint32_t j = 0; j < inst->sectorsPerCluster; ++j) {
                storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + inst->maxRootDirEntries/16 + (cur_clust - 2)*inst->sectorsPerCluster + j, cur_sec_bytes, 512);
                for(uint16_t i = 0; i < 512; i += 32) {
                    
                    if ((cur_sec_bytes[i] == 0x00) || (cur_sec_bytes[i] == 0xE5)){
                        if (is_long_fname) {
                            for(uint16_t l = i; l < 512; l += 32){
                                for(uint8_t k = 0; k < 32; ++k){
                                    cur_sec_bytes[l+k] = long_fname_entry_buffer[long_fname_entry_index * 32 + k];
                                }
                                ++long_fname_entry_index;
                                if (long_fname_entry_index == long_fname_entry_count) break; //Everything has been written
                            }
                            storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT * inst->FATCopies + inst->maxRootDirEntries/16 + (cur_clust - 2) * inst->sectorsPerCluster + j, cur_sec_bytes, 512);
                            
                            if (long_fname_entry_index == long_fname_entry_count){
                                //Everything has been written
                                free(long_fname_entry_buffer);
                                return entry;
                            }
                        } else {
                            uint16_t k;
                            for(k = 0; k < MIN(/*new_filename.Length*/lastDot, 8); ++k){
                                entry[k] = new_filename[k];
                            }
                            //Fill in the spaces for the file name
                            for(; k < 8; ++k){
                                entry[k] = 0x20;
                            }
                            for(k = 0; k < MIN(/*new_fileext.Length*/cur_file_len-lastDot, 3); ++k){
                                entry[8 + k] = new_fileext[k];
                            }
                            //Fill in the spaces for the file extension
                            for(; k < 3; ++k){
                                entry[8 + k] = 0x20;
                            }
                            
                            entry[11] = attribute;
                            entry[12] = 0x00;
                            entry[13] = 0x00; //TODO: Time and date values
                            entry[14] = 0x00;
                            entry[15] = 0x00;
                            entry[0x10] = 0x00;
                            entry[0x11] = 0x00;
                            entry[0x12] = 0x00;
                            entry[0x13] = 0x00;
                            entry[0x16] = 0x00;
                            entry[0x17] = 0x00;
                            entry[0x18] = 0x00;
                            entry[0x19] = 0x00;
                            entry[0x1C] = 0x00; //Filesize
                            entry[0x1D] = 0x00;
                            entry[0x1E] = 0x00;
                            entry[0x1F] = 0x00;
                            entry[0x14] = 0x00; //First cluster
                            entry[0x15] = 0x00;
                            entry[0x1A] = 0x00;
                            entry[0x1B] = 0x00;
                            
                            for(k = 0; k < 32; ++k){
                                cur_sec_bytes[i+k] = entry[k];
                            }
                            storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT * inst->FATCopies + inst->maxRootDirEntries/16 + (cur_clust - 2) * inst->sectorsPerCluster + j, cur_sec_bytes, 512);
                            return entry;
                        }
                    }
                }
            }
            prev_clust = cur_clust;
            cur_clust = FAT_FATgetCluster(inst, cur_clust);
            //create a new cluster of directory if previous one is full
            if(cur_clust == 0){
                cur_clust = FAT_FATfindFreeCluster(inst);
                if(cur_clust == 0){
                    if(is_long_fname) free(long_fname_entry_buffer);
                    return 0; //return NULL if there is no free cluster
                }
                FAT_FATsetCluster(inst, prev_clust, cur_clust);
                FAT_FATsetCluster(inst, cur_clust, (inst->fileSys == FAT12) ? 0x0FF8 : ((inst->fileSys == FAT16) ? 0xFFF8 : 0x0FFFFFF8));
            }
        }
        
    }
    
    if(is_long_fname) free(long_fname_entry_buffer);
    return 0;
}

void FAT_setVolumeLabel(FAT* inst, char* volumeLabel){
    stringReplace("/", " ", volumeLabel);
    stringReplace("\\", " ", volumeLabel);
    
    if(inst->fileSys <= FAT16){
        for(uint32_t j = 0; j < inst->maxRootDirEntries/16; ++j){
            uint8_t cur_sec_bytes[512];
            storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + j, cur_sec_bytes, 512);
            for(uint32_t i = 0; i < 512; i += 32){
                if(cur_sec_bytes[i + 11] == FAT_ATTR_VOLLABEL){
                    //Entry is a volume label
                    uint8_t k;
                    for(k = 0; k < 11; ++k){
                        if(volumeLabel[k] == 0) break;
                        cur_sec_bytes[i+k] = volumeLabel[k];
                    }
                    for(; k < 11; ++k){
                        cur_sec_bytes[i+k] = 0x20;
                    }
                    storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT * inst->FATCopies + j, cur_sec_bytes, 512);
                    return;
                }
            }
        }
    } else {
        uint32_t cur_clust = inst->startClustOfRootDir;
        
        while(cur_clust != 0){
            for(uint32_t j = 0; j < inst->sectorsPerCluster; ++j){
                uint8_t cur_sec_bytes[512];
                storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + (cur_clust - 2)*inst->sectorsPerCluster + j, cur_sec_bytes, 512);
                for(uint32_t i = 0; i < 512; i += 32){
                    if(cur_sec_bytes[i + 11] == FAT_ATTR_VOLLABEL){
                        //Entry is a volume label
                        uint8_t k;
                        for(k = 0; k < 11; ++k){
                            if(volumeLabel[k] == 0) break;
                            cur_sec_bytes[i+k] = volumeLabel[k];
                        }
                        for(; k < 11; ++k){
                            cur_sec_bytes[i+k] = 0x20;
                        }
                        storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT * inst->FATCopies + (cur_clust - 2) * inst->sectorsPerCluster + j, cur_sec_bytes, 512);
                        return;
                    }
                }
            }
            cur_clust = FAT_FATgetCluster(inst, cur_clust);
        }
    }
    
    //If there is no Volume Label, create one
    char temp_volumeLabel[13];
    uint8_t k;
    for(k = 0; k < 13; ++k){
        temp_volumeLabel[k] = 0;
    }
    uint8_t volLabel_len = strlen(volumeLabel);
    for(k = 0; k < MIN(volLabel_len, 8); ++k){
        temp_volumeLabel[k] = volumeLabel[k];
    }
    if(volLabel_len >= 9) temp_volumeLabel[8] = '.';
    for(k = 8; k < MIN(volLabel_len, 11); ++k){
        temp_volumeLabel[k+1] = volumeLabel[k];
    }
    free(FAT_createFile(inst, temp_volumeLabel, FAT_ATTR_VOLLABEL));
}

uint8_t FAT_readFileEntryByIndex(FAT* inst, file_t* file_inst, char* filepath, uint32_t index){
    uint8_t ret[32];
    //Format and split the file path
    char spl_path[FAT_MAX_PATH_FILES][FAT_MAX_FNAME_LEN];
    for(uint32_t k = 0; k < FAT_MAX_PATH_FILES; ++k){
        for(uint32_t l = 0; l < FAT_MAX_FNAME_LEN; ++l){
            spl_path[k][l] = 0;
        }
    }
    char delimiter[] = "/";
    int path_size = 0;
    formatFilePath(filepath);
    size_t tmp_filepath_len = strlen(filepath)+1;
    char tmp_filepath[tmp_filepath_len];
    memset(tmp_filepath, 0, tmp_filepath_len);
    strcpy(tmp_filepath, filepath);
    char* cur_file = strtok(tmp_filepath, delimiter);
    while(cur_file != 0 && path_size < FAT_MAX_PATH_FILES){
        if(strlen(cur_file) > FAT_MAX_FNAME_LEN-1) break; //make sure current filename is not too long
        strcpy(spl_path[path_size], cur_file);
        cur_file = strtok(0, delimiter);
        ++path_size;
    }
    
    uint32_t cur_clust = inst->startClustOfRootDir;
    
    for(uint32_t k = 0; k < path_size+1; ++k){
        int32_t cur_file_len = 0;
        int32_t lastDot = 0;
        if(k < path_size) {
            //Split the current file into name and extension
            cur_file_len = strlen(spl_path[k]);
            lastDot = strrchr(spl_path[k], '.');
            if(lastDot == -1) lastDot = cur_file_len;
        }
        char cur_file_name[lastDot+1];
        char cur_file_ext[cur_file_len-lastDot];
        if(k < path_size) {
            for(uint32_t j = 0; j < lastDot; ++j){
                cur_file_name[j] = spl_path[k][j];
            }
            cur_file_name[lastDot] = 0; //Null termination
            
            for(uint32_t j = (lastDot+1); j < cur_file_len; ++j){
                cur_file_ext[j-(lastDot+1)] = spl_path[k][j];
            }
            cur_file_ext[MAX(cur_file_len-lastDot-1, 0)] = 0; //Null termination
        }
        
        char long_fname_buffer[208];
        //Clear long_fname_buffer
        for(uint8_t l = 0; l < 208; ++l){
            long_fname_buffer[l] = 0;
        }
        
        if((inst->fileSys <= FAT16) && k == 0){ //root directory of FAT12 and FAT16
            uint32_t cur_index = 0;
            uint8_t loopBreak = 0;
            for(uint8_t j = 0; j < inst->maxRootDirEntries/16; ++j){
                //Read current sector
                uint8_t cur_sec_bytes[512];
                storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + j, cur_sec_bytes, 512);
                
                for(uint32_t i = 0; i < 512; i += 32){
                    if(cur_sec_bytes[i] == 0x00){
                        return 0; //Free from here on
                    }
                    if(cur_sec_bytes[i] == 0xE5) continue; //File was deleted
                    
                    if(cur_sec_bytes[i+11] == 0x0F) { //Part of long filename
                        uint8_t cur_long_fname_temp[] = {cur_sec_bytes[i + 1], cur_sec_bytes[i + 3], cur_sec_bytes[i + 5], cur_sec_bytes[i + 7], cur_sec_bytes[i + 9],
                                    cur_sec_bytes[i + 14], cur_sec_bytes[i + 16], cur_sec_bytes[i + 18], cur_sec_bytes[i + 20], cur_sec_bytes[i + 22], cur_sec_bytes[i + 24],
                                    cur_sec_bytes[i + 28], cur_sec_bytes[i + 30]};
                        for(uint8_t l = 0; l < /*Length of cur_long_fname_temp*/13; ++l){
                            if ((/*if last long file name entry*/(cur_sec_bytes[i] & 0xF0) == 0x40) && (cur_long_fname_temp[l] == 0)) break;
                            long_fname_buffer[((cur_sec_bytes[i] & 0x0F)-1)*13+l] = (char)cur_long_fname_temp[l];
                        }
                        continue;
                    }
                    
                    char cur_filename[FAT_MAX_FNAME_LEN];
                    char cur_ext[FAT_MAX_FNAME_LEN];
                    
                    if(long_fname_buffer[0] == 0){ //if file name is short
                        for(uint8_t l = 0; l < 8; ++l){
                            cur_filename[l] = (char)cur_sec_bytes[i+l];
                        }
                        cur_filename[8] = 0; //Null termination
                        strtrimend(cur_filename);
                        for(uint8_t l = 0; l < 3; ++l){
                            cur_ext[l] = (char)cur_sec_bytes[i+8+l];
                        }
                        cur_ext[3] = 0; //Null termination
                        strtrimend(cur_ext);
                    }else{ //if file name is long
                        //Split the current file into name and extension
                        cur_file_len = strlen(long_fname_buffer);
                        lastDot = strrchr(long_fname_buffer, '.');
                        if(lastDot == -1) lastDot = cur_file_len;
                        cur_filename[1] = 0; //Null termination if lastDot = 0
                        for(uint32_t l = 0; l < lastDot; ++l){
                            cur_filename[l] = long_fname_buffer[l];
                        }
                        cur_filename[lastDot] = 0; //Null termination
                        for(uint32_t l = (lastDot+1); l < cur_file_len; ++l){
                            cur_ext[l-(lastDot+1)] = long_fname_buffer[l];
                        }
                        cur_ext[cur_file_len-lastDot-1] = 0; //Null termination
                        
                        //Clear long_fname_buffer
                        for(uint8_t l = 0; l < 208; ++l){
                            long_fname_buffer[l] = 0;
                        }
                    }
                    
                    if(k == path_size){
                        if(cur_index != index){
                            ++cur_index;
                            continue;
                        }
                        file_inst->size = cur_sec_bytes[i+0x1C] + (cur_sec_bytes[i+0x1D] << 8) + (cur_sec_bytes[i+0x1E] << 16) + (cur_sec_bytes[i+0x1F] << 24);
                        memset(file_inst->name, 0, 35);
                        strcpy(file_inst->name, cur_filename);
                        if(cur_ext[0] != 0){
                            strcat(file_inst->name, ".");
                            strcat(file_inst->name, cur_ext);
                        }
                        file_inst->attribute = cur_sec_bytes[i+11];
                        return 1;
                    } else {
                        tolower(cur_filename);
                        tolower(cur_file_name);
                        if(strcmp(cur_filename, cur_file_name) == 0){ //if the requested file name matches the file name of the current entry
                            if(*cur_file_ext == 0){ //if cur_file_ext is empty
                                //Copy the current file entry to ret
                                for(uint8_t l = 0; l < 32; ++l){
                                    ret[l] = cur_sec_bytes[i+l];
                                }
                            }else{
                                tolower(cur_ext);
                                tolower(cur_file_ext);
                                if(strcmp(cur_ext, cur_file_ext) == 0){ //if the requested file extension matches the file extension of the current entry
                                    //Copy the current file entry to ret
                                    for(uint8_t l = 0; l < 32; ++l){
                                        ret[l] = cur_sec_bytes[i+l];
                                    }
                                }
                                else continue;
                            }
                            
                            if(k == (path_size-1)){ //if is final file
                                cur_clust = ret[0x1A] + (ret[0x1B] << 8) + (ret[0x14] << 16) + (ret[0x15] << 24);
                                loopBreak = 1;
                                break;
                            }else{
                                if((ret[11] & FAT_ATTR_SUBDIR) == FAT_ATTR_SUBDIR){ //Subdirectory
                                    cur_clust = ret[0x1A] + (ret[0x1B] << 8) + (ret[0x14] << 16) + (ret[0x15] << 24);
                                    loopBreak = 1;
                                    break;
                                }else{
                                    return 0;
                                }
                            }
                            
                        }
                    }
                }
                if(loopBreak) break;
            }
            
        } else {
            uint32_t cur_index = 0;
            while(cur_clust != 0){
                uint8_t loopBreak = 0;
                for(uint32_t j = 0; j < inst->sectorsPerCluster; ++j){
                    //Read current sector
                    uint8_t cur_sec_bytes[512];
                    storage_readSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + inst->maxRootDirEntries/16 + (cur_clust - 2)*inst->sectorsPerCluster + j, cur_sec_bytes, 512);
                    
                    for(uint32_t i = 0; i < 512; i += 32){
                        if(cur_sec_bytes[i] == 0x00){
                            return 0; //Free from here on
                        }
                        if(cur_sec_bytes[i] == 0xE5) continue; //File was deleted
                        
                        if(cur_sec_bytes[i+11] == 0x0F) { //Part of long filename
                            uint8_t cur_long_fname_temp[] = {cur_sec_bytes[i + 1], cur_sec_bytes[i + 3], cur_sec_bytes[i + 5], cur_sec_bytes[i + 7], cur_sec_bytes[i + 9],
                                        cur_sec_bytes[i + 14], cur_sec_bytes[i + 16], cur_sec_bytes[i + 18], cur_sec_bytes[i + 20], cur_sec_bytes[i + 22], cur_sec_bytes[i + 24],
                                        cur_sec_bytes[i + 28], cur_sec_bytes[i + 30]};
                            for(uint8_t l = 0; l < /*Length of cur_long_fname_temp*/13; ++l){
                                if ((/*if last long file name entry*/(cur_sec_bytes[i] & 0xF0) == 0x40) && (cur_long_fname_temp[l] == 0)) break;
                                long_fname_buffer[((cur_sec_bytes[i] & 0x0F)-1)*13+l] = (char)cur_long_fname_temp[l];
                            }
                            continue;
                        }
                        
                        char cur_filename[FAT_MAX_FNAME_LEN];
                        char cur_ext[FAT_MAX_FNAME_LEN] = {0};
                        
                        if(long_fname_buffer[0] == 0){ //if file name is short
                            for(uint8_t l = 0; l < 8; ++l){
                                cur_filename[l] = (char)cur_sec_bytes[i+l];
                            }
                            cur_filename[8] = 0; //Null termination
                            strtrimend(cur_filename);
                            for(uint8_t l = 0; l < 3; ++l){
                                cur_ext[l] = (char)cur_sec_bytes[i+8+l];
                            }
                            cur_ext[3] = 0; //Null termination
                            strtrimend(cur_ext);
                        }else{ //if file name is long
                            //Split the current file into name and extension
                            cur_file_len = strlen(long_fname_buffer);
                            lastDot = strrchr(long_fname_buffer, '.');
                            if(lastDot == -1) lastDot = cur_file_len;
                            cur_filename[1] = 0; //Null termination if lastDot = 0
                            for(uint32_t l = 0; l < lastDot; ++l){
                                cur_filename[l] = long_fname_buffer[l];
                            }
                            cur_filename[lastDot] = 0; //Null termination
                            for(uint32_t l = (lastDot+1); l < cur_file_len; ++l){
                                cur_ext[l-(lastDot+1)] = long_fname_buffer[l];
                            }
                            cur_ext[cur_file_len-lastDot-1] = 0; //Null termination
                            
                            //Clear long_fname_buffer
                            for(uint8_t l = 0; l < 208; ++l){
                                long_fname_buffer[l] = 0;
                            }
                        }
                        
                        if(k == path_size){
                            if(cur_index != index){
                                ++cur_index;
                                continue;
                            }
                            file_inst->size = cur_sec_bytes[i+0x1C] + (cur_sec_bytes[i+0x1D] << 8) + (cur_sec_bytes[i+0x1E] << 16) + (cur_sec_bytes[i+0x1F] << 24);
                            memset(file_inst->name, 0, 35);
                            strcpy(file_inst->name, cur_filename);
                            if(cur_ext[0] != 0){
                                strcat(file_inst->name, ".");
                                strcat(file_inst->name, cur_ext);
                            }
                            file_inst->attribute = cur_sec_bytes[i+11];
                            return 1;
                        } else {
                            tolower(cur_filename);
                            tolower(cur_file_name);
                            if(strcmp(cur_filename, cur_file_name) == 0){ //if the requested file name matches the file name of the current entry
                                if(*cur_file_ext == 0){ //if cur_file_ext is empty
                                    //Copy the current file entry to ret
                                    for(uint8_t l = 0; l < 32; ++l){
                                        ret[l] = cur_sec_bytes[i+l];
                                    }
                                }else{
                                    tolower(cur_ext);
                                    tolower(cur_file_ext);
                                    if(strcmp(cur_ext, cur_file_ext) == 0){ //if the requested file extension matches the file extension of the current entry
                                        //Copy the current file entry to ret
                                        for(uint8_t l = 0; l < 32; ++l){
                                            ret[l] = cur_sec_bytes[i+l];
                                        }
                                    }
                                    else continue;
                                }
                                
                                if(k == (path_size-1)){ //if is final file
                                    cur_clust = ret[0x1A] + (ret[0x1B] << 8) + (ret[0x14] << 16) + (ret[0x15] << 24);
                                    loopBreak = 1;
                                    break;
                                }else{
                                    if((ret[11] & FAT_ATTR_SUBDIR) == FAT_ATTR_SUBDIR){ //Subdirectory
                                        cur_clust = ret[0x1A] + (ret[0x1B] << 8) + (ret[0x14] << 16) + (ret[0x15] << 24);
                                        loopBreak = 1;
                                        break;
                                    }else{
                                        return 0;
                                    }
                                }
                                
                            }
                        }
                    }
                    if(loopBreak) break;
                }
                if(loopBreak) break;
                cur_clust = FAT_FATgetCluster(inst, cur_clust);
            }
        }
    }
    return 0;
}

uint8_t FAT_format(FAT* inst, uint8_t fileSys, char* volumeLabel, uint16_t reservedSectors, uint8_t quick){
    switch(fileSys){
        case FAT12:
        {
            if (inst->lbaLength <= 4096) inst->sectorsPerCluster = 1; //0 Bytes - 2 MB
            else if (inst->lbaLength <= 8192) inst->sectorsPerCluster = 2; //2 MB - 4 MB
            else if (inst->lbaLength <= 16384) inst->sectorsPerCluster = 4; //4 MB - 8 MB
            else if (inst->lbaLength <= 32768) inst->sectorsPerCluster = 8; //8 MB - 16 MB
            else if (inst->lbaLength <= 65536) inst->sectorsPerCluster = 16; //16 MB - 32 MB
            else if (inst->lbaLength <= 131072) inst->sectorsPerCluster = 32; //32 MB - 64 MB
            else if (inst->lbaLength <= 262144) inst->sectorsPerCluster = 64; //64 MB - 128 MB
            else inst->sectorsPerCluster = 128; //128 MB - 256 MB
            inst->FATCopies = 2;
            inst->maxRootDirEntries = 224;
            inst->sectorsPerFAT = (uint32_t)((inst->lbaLength - reservedSectors - inst->maxRootDirEntries/16) / ((512/1.5) * inst->sectorsPerCluster + 2) + 0.998 /*Makes it round up*/);
            if ((reservedSectors == 0) || ((reservedSectors+inst->sectorsPerFAT*inst->FATCopies) > inst->lbaLength)) return false; //Not enough space in this partition
            inst->reservedSectors = reservedSectors;
            inst->startClustOfRootDir = 0; // unused in FAT12
            
            //Bios Parameter Block
            uint8_t cur_sec[] = { 0xEB, 0x58, 0x90, 0x43, 0x68, 0x61, 0x4F, 0x53, 0x31, 0x2E, 0x30, 0x00, 0x02, inst->sectorsPerCluster, (uint8_t)reservedSectors, (uint8_t)(reservedSectors >> 8),
            inst->FATCopies, (uint8_t)inst->maxRootDirEntries, (uint8_t)(inst->maxRootDirEntries >> 8), 0x00, 0x00, 0xF8, (uint8_t)inst->sectorsPerFAT, (uint8_t)(inst->sectorsPerFAT >> 8), 0x3F, 0x00, 0x10, 0x00, (uint8_t)inst->lbaStart, (uint8_t)(inst->lbaStart >> 8), (uint8_t)(inst->lbaStart >> 16), (uint8_t)(inst->lbaStart >> 24),
            (uint8_t)inst->lbaLength, (uint8_t)(inst->lbaLength >> 8), (uint8_t)(inst->lbaLength >> 16), (uint8_t)(inst->lbaLength >> 24), 0x00, 0x00, 0x29, /*(uint8_t)random.Next(0xFF), (uint8_t)random.Next(0xFF), (uint8_t)random.Next(0xFF), (uint8_t)random.Next(0xFF)*/0x12, 0x34, 0x56, 0x78, 0x20, 0x20, 0x20, 0x20, 0x20,
            0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 'F', 'A', 'T', '1', '2', 0x20, 0x20, 0x20, /*Boot code start*/0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /*Boot code end*/, 0x55, 0xAA };
            
            storage_writeSector(inst->storageDev, inst->lbaStart, cur_sec, 512);
            
            //Clear FAT and root directory
            memset(cur_sec, 0, 512);
            for(uint32_t j = 0; j < (inst->sectorsPerFAT*inst->FATCopies + inst->maxRootDirEntries/16); ++j){
                storage_writeSector(inst->storageDev, inst->lbaStart+inst->reservedSectors + j, cur_sec, 512);
            }
            
            //Write first two clusters into FAT
            FAT_FATsetCluster(inst, 0, 0xFF8);
            FAT_FATsetCluster(inst, 1, 0xFFF);
            
            //Volume label
            if(strlen(volumeLabel) != 0){
                FAT_setVolumeLabel(inst, volumeLabel);
            }
            
            //If not quick then clear all clusters
            if(!quick){
                uint8_t nul_secs[512*16] = {0};
                uint32_t cur_time = timer_getMilliseconds();
                uint32_t prev_time;
                uint32_t last_i_measure = 0;
                for(uint32_t i = inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT * inst->FATCopies + inst->maxRootDirEntries/16; i < inst->lbaLength; ++i){
                    if((i+16) < inst->lbaLength){
                        storage_writeSector(inst->storageDev, i, nul_secs, 512*16);
                        i += 15;
                    } else {
                        storage_writeSector(inst->storageDev, i, cur_sec, 512);
                    }
                    if((i % 128) == 0){
                        if(timer_getMilliseconds() > (cur_time+1000)){
                            prev_time = cur_time;
                            cur_time = timer_getMilliseconds();
                            //if((cur_time-prev_time) != 0){
                                printf("%d KByte/s  \r",(((i-last_i_measure)*512)/(cur_time-prev_time)));
                                last_i_measure = i;
                            //}
                        }
                    }
                }
                putch('\n');
            }
            
            return 1;
        }
        case FAT16:
            return 1; //TODO: Formating for FAT16
        default:
        {
            if (inst->lbaLength <= 524288) inst->sectorsPerCluster = 1; //0 Bytes - 256 MB
            else if (inst->lbaLength <= 1048576) inst->sectorsPerCluster = 2; //256 MB - 512 MB
            else if (inst->lbaLength <= 2097152) inst->sectorsPerCluster = 4; //512 MB - 1 GB
            else if (inst->lbaLength <= 16777216) inst->sectorsPerCluster = 8; //1 GB - 8 GB
            else if (inst->lbaLength <= 33554432) inst->sectorsPerCluster = 16; //8 GB - 16 GB
            else if (inst->lbaLength <= 67108864) inst->sectorsPerCluster = 32; //16 GB - 32 GB
            else if (inst->lbaLength <= 4294967296) inst->sectorsPerCluster = 64; //32 GB - 2 TB
            else inst->sectorsPerCluster = 128; //1 TB - 16 TB
            inst->FATCopies = 2;
            inst->sectorsPerFAT = (uint32_t)((inst->lbaLength - reservedSectors) / (128 * inst->sectorsPerCluster + 2) + 0.993 /*Makes it round up*/);
            if ((reservedSectors < 9) || ((reservedSectors+inst->sectorsPerFAT*inst->FATCopies) > inst->lbaLength)) return false; //Not enough space in this partition
            inst->reservedSectors = reservedSectors;
            inst->startClustOfRootDir = 2;
            
            //Bios Parameter Block
            uint8_t cur_sec[] = { 0xEB, 0x58, 0x90, 0x43, 0x68, 0x61, 0x4F, 0x53, 0x31, 0x2E, 0x30, 0x00, 0x02, inst->sectorsPerCluster, (uint8_t)reservedSectors, (uint8_t)(reservedSectors >> 8),
            inst->FATCopies, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x3F, 0x00, 0x10, 0x00, (uint8_t)inst->lbaStart, (uint8_t)(inst->lbaStart >> 8), (uint8_t)(inst->lbaStart >> 16), (uint8_t)(inst->lbaStart >> 24),
            (uint8_t)inst->lbaLength, (uint8_t)(inst->lbaLength >> 8), (uint8_t)(inst->lbaLength >> 16), (uint8_t)(inst->lbaLength >> 24), (uint8_t)inst->sectorsPerFAT, (uint8_t)(inst->sectorsPerFAT >> 8), (uint8_t)(inst->sectorsPerFAT >> 16), (uint8_t)(inst->sectorsPerFAT >> 24), 0x00, 0x00, 0x00, 0x00, (uint8_t)inst->startClustOfRootDir, (uint8_t)(inst->startClustOfRootDir >> 8), (uint8_t)(inst->startClustOfRootDir >> 16), (uint8_t)(inst->startClustOfRootDir >> 24),
            /*FSInfo sector*/0x01, 0x00/**/, /*Backup boot sector*/0x06, 0x00/**/, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x80, 0x00, 0x29, /*(uint8_t)random.Next(0xFF), (uint8_t)random.Next(0xFF), (uint8_t)random.Next(0xFF), (uint8_t)random.Next(0xFF)*/0x12, 0x34, 0x56, 0x78, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
            0x20, 0x20, 0x46, 0x41, 0x54, 0x33, 0x32, 0x20, 0x20, 0x20, /*Boot code start*/0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /*Boot code end*/, 0x55, 0xAA };
            
            storage_writeSector(inst->storageDev, inst->lbaStart, cur_sec, 512);
            storage_writeSector(inst->storageDev, inst->lbaStart+6, cur_sec, 512);
            
            //FSInfo sector
            cur_sec[0] = 0x52;
            cur_sec[1] = 0x52;
            cur_sec[2] = 0x61;
            cur_sec[3] = 0x41;
            uint16_t i;
            for (i = 4; i < 484; ++i)
            {
                cur_sec[i] = 0;
            }
            cur_sec[0x1E4] = 0x72;
            cur_sec[0x1E5] = 0x72;
            cur_sec[0x1E6] = 0x41;
            cur_sec[0x1E7] = 0x61;
            //Number of free clusters
            cur_sec[0x1E8] = 0xFF;
            cur_sec[0x1E9] = 0xFF;
            cur_sec[0x1EA] = 0xFF;
            cur_sec[0x1EB] = 0xFF;
            //The most recent free cluster
            cur_sec[0x1EC] = 0xFF;
            cur_sec[0x1ED] = 0xFF;
            cur_sec[0x1EE] = 0xFF;
            cur_sec[0x1EF] = 0xFF;
            for (i = 0x1F0; i < 0x1FE; ++i)
            {
                cur_sec[i] = 0;
            }
            //FSInfo sector signature
            cur_sec[0x1FE] = 0x55;
            cur_sec[0x1FF] = 0xAA;
            
            storage_writeSector(inst->storageDev, inst->lbaStart+1, cur_sec, 512);
            storage_writeSector(inst->storageDev, inst->lbaStart+7, cur_sec, 512);
            
            for (i = 0; i < 0x1FE; ++i)
            {
                cur_sec[i] = 0;
            }
            storage_writeSector(inst->storageDev, inst->lbaStart+2, cur_sec, 512);
            storage_writeSector(inst->storageDev, inst->lbaStart+8, cur_sec, 512);
            
            cur_sec[0x1FE] = 0;
            cur_sec[0x1FF] = 0;
            
            storage_writeSector(inst->storageDev, inst->lbaStart+3, cur_sec, 512);
            storage_writeSector(inst->storageDev, inst->lbaStart+4, cur_sec, 512);
            storage_writeSector(inst->storageDev, inst->lbaStart+5, cur_sec, 512);
            for (i = 9; i < inst->reservedSectors; ++i)
            {
                storage_writeSector(inst->storageDev, inst->lbaStart+i, cur_sec, 512);
            }
            
            //Clear FAT
            for(uint32_t j = 0; j < inst->sectorsPerFAT*inst->FATCopies; ++j){
                storage_writeSector(inst->storageDev, inst->lbaStart+inst->reservedSectors + j, cur_sec, 512);
            }
            
            //Write first two clusters into FAT
            FAT_FATsetCluster(inst, 0, 0x0FFFFFF8);
            FAT_FATsetCluster(inst, 1, 0x0FFFFFFF);
            
            //Write first root directory cluster into FAT
            FAT_FATsetCluster(inst, inst->startClustOfRootDir, 0x0FFFFFFF);
            
            //Clear first root directory cluster
            for(uint8_t j = 0; j < inst->sectorsPerCluster; ++j){
                storage_writeSector(inst->storageDev, inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT*inst->FATCopies + (inst->startClustOfRootDir - 2) * inst->sectorsPerCluster + j, cur_sec, 512);
            }
            
            //Volume label
            if(strlen(volumeLabel) != 0){
                FAT_setVolumeLabel(inst, volumeLabel);
            }
            
            //If not quick then clear all clusters
            if(!quick){
                uint32_t cur_time = timer_getMilliseconds();
                uint32_t prev_time;
                uint32_t last_i_measure = 0;
                for(uint32_t i = inst->lbaStart + inst->reservedSectors + inst->sectorsPerFAT * inst->FATCopies + (inst->startClustOfRootDir - 1) * inst->sectorsPerCluster; i < inst->lbaLength; ++i){
                    storage_writeSector(inst->storageDev, i, cur_sec, 512);
                    /*if((i % 1024) == 0){
                        uint32_t prev_time = cur_time;
                        cur_time = timer_getMilliseconds();
                        if((cur_time-prev_time) != 0){
                            printf("%d KByte/s  ",(512000/(cur_time-prev_time)));
                            move_cursor_home();
                        }
                    }*/
                    if((i % 128) == 0){
                        if(timer_getMilliseconds() > (cur_time+1000)){
                            prev_time = cur_time;
                            cur_time = timer_getMilliseconds();
                            //if((cur_time-prev_time) != 0){
                                printf("%d KByte/s  \r",(((i-last_i_measure)*512)/(cur_time-prev_time)));
                                last_i_measure = i;
                            //}
                        }
                    }
                }
                putch('\n');
            }
            
            return 1;
        }
    }
}

uint8_t FAT_abstract_createFile(FAT* inst, char* filepath, file_t* file_inst){
    uint8_t* entry = FAT_createFile(inst, filepath, FAT_ATTR_ARCHIVE);
    file_inst->size = 0;
    if(entry == 0){
        memset(file_inst->data, 0, 32);
        file_inst->attribute = 0;
        return 0; // Operation failed
    }
    memcpy(file_inst->data, entry, 32);
    file_inst->attribute = entry[0x11];
    free(entry);
    return 1; // Operation succeeded
}

uint8_t FAT_abstract_findFile(FAT* inst, char* filepath, file_t* file_inst){
    uint8_t* entry = FAT_readFileEntry(inst, filepath);
    if(entry == 0){
        memset(file_inst->data, 0, 32);
        file_inst->size = 0;
        file_inst->attribute = 0;
        return 0; // Operation failed
    }
    memcpy(file_inst->data, entry, 32);
    file_inst->size = FAT_getFilesize(entry);
    file_inst->attribute = entry[0x11];
    return 1; // Operation succeeded
}

void FAT_abstract_readFileContents(FAT* inst, file_t* file_inst, uint8_t* buffer, uint32_t start, uint32_t len){
    FAT_readFileContents(inst, file_inst->data, buffer, start, len);
}

uint8_t FAT_abstract_isDirectory(FAT* inst, char* filepath){
    if(filepath[0] == 0) return 1; // Root directory
    uint8_t* entry = FAT_readFileEntry(inst, filepath);
    if(entry == 0) return 0;
    if(entry[11] & FAT_ATTR_SUBDIR) return 1;
    return 0;
}

uint8_t FAT_abstract_createDirectory(FAT* inst, char* filepath){
    return FAT_createDirectory(inst, filepath);
}
