#include <stdio.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "mkdosfs.h"

#define FAT12 0
#define FAT16 1
#define FAT32 2

#define ROOT_SECTS 14

FILE *fp, *sfp, *mfp;

uint8_t buffer[512];     // a temp buffer
char strbuff[80];        // a temp str buffer
char filename[80];       // filename
char bootname[80];       // boot filename
char mbrfile[80];
char volumelabel[11];

int sec_res = -1;        // reserved sectors
int bpbz = 0;            // size of used BPB
int spfat;               // sectors per fat
uint32_t sectors = 2880; // total sectors
bool make_mbr = false;   // do as a MBR and a partition?
bool make_hd = false;    // make hard drive image
bool make_fd = false;    // make floppy drive image
int image_size = -1;     // image size in megabytes (if hd used)
int spt = -1;            // spt (if fd used)
int heads = -1;          // heads     "
int cyl = -1;            // cyliders  "
int spc = -1;            // sectors per cluster
int fatz = -1;           // fat size (12, 16, 32)
int fats = -1;           // fats (1 or 2)
bool filenamegiven = false; // was a filename given on the command line
bool bootnamegiven = false; // was a boot filename given on the command line
bool mbrfilegiven = false;
bool volumelabelgiven = false;
bool make_mbr_given = false;

bool verbose = true;

uint8_t media_descriptor(bool ishd, int heads, int spt, int cylinders);
int get_params(int argc, char *argv[]);
void write_sector(FILE *fp, void *ptr);
void putdot();

int main(int argc, char *argv[]) {

    int i, j, r;
    uint32_t last_cnt;
    uint8_t boot_sector[512];

    // get the command line parameters
    if (get_params(argc, argv) != 0)
        return 1;

    if (!make_hd && !make_fd) {
        do {
            printf("Make Floppy or Hard Drive image? (fd/hd) [fd]: ");
            fgets(strbuff, sizeof(strbuff), stdin);
            if (!strlen(strbuff)) {
                make_fd = true;
                make_hd = false;
                break;
            }
        } while (strcmp(strbuff, "fd") && strcmp(strbuff, "hd"));
        if (strlen(strbuff))
            if (!strcmp(strbuff, "fd")) {
                make_fd = true;
                make_hd = false;
            } else {
                make_fd = false;
                make_hd = true;
            }
    }

    if (!make_mbr_given && make_hd) {
        do {
            printf("Create MBR and separate partition? (y/n) [y]: ");
            fgets(strbuff, sizeof(strbuff), stdin);
            if (!strlen(strbuff)) {
                make_mbr = true;
                break;
            }
        } while (strcmp(strbuff, "y") && strcmp(strbuff, "n"));
        if (!strcmp(strbuff, "y"))
            make_mbr = true;
    }

    if (!filenamegiven) {
        printf("                          As filename [%c.img]: ", (make_hd) ? 'c' : 'a');
        fgets(filename, sizeof(filename), stdin);
        if (!strlen(filename))
            strcpy(filename, (make_hd) ? "c.img" : "a.img");
        if (!strchr(filename, '.'))
            strcat(filename, ".img");
    }

    if (make_hd) {
        if (image_size == -1) {
            do {
                printf("                   Size (meg) (1 - 1024) [10]: ");
                fgets(strbuff, sizeof(strbuff), stdin);
                if (!strlen(strbuff))
                    i = 10;
                else
                    i = atoi(strbuff);
            } while ((i < 1) || (i > 1024));
            cyl = (i << 1);  // very close at 16 heads and 63 spt
        }
        else {
            if ((image_size > 0) && (image_size <= 1024))
                cyl = (image_size << 1);
            else {
                printf("Illegal value for Image Size: %i\n", image_size);
                return 1;
            }
        }
        heads = 16;
        spt = 63;
    }
    else {
        if (spt == -1) {
            do {
                printf("   Sectors per track (8, 9, 15, 18, 21, 36)  [18]: ");
                fgets(strbuff, sizeof(strbuff), stdin);
                if (!strlen(strbuff))
                    i = 18;
                else
                    i = atoi(strbuff);
            } while ((i != 8) && (i != 9) && (i != 15) && (i != 18) && (i != 21) && (i != 36));

            if (i == 36)
                spc = 2;  // default

            spt = i;
        }
        else {
            if ((spt != 8) && (spt != 9) && (spt != 15) && (spt != 18) && (spt != 21) && (spt != 36)) {
                printf(" Illegal value for SPT: %i\n", spt);
                return 1;
            }
        }
        if (heads == -1) {
            do {
                printf("                           Heads: (1, 2)  [2]: ");
                fgets(strbuff, sizeof(strbuff), stdin);

                if (!strlen(strbuff))
                    i = 2;
                else
                    i = atoi(strbuff);
            } while ((i != 1) && (i != 2));
            heads = i;
        }
        else {
            if ((heads != 1) && (heads != 2)) {
                printf(" Illegal value for heads: %i\n", heads);
                return 1;
            }
        }
        if (cyl == -1) {
            do {
                if ((spt < 9) || (heads == 1)) {
                    printf("                    Cylinders: (40, 80)  [40]: ");
                    fgets(strbuff, sizeof(strbuff), stdin);

                    if (!strlen(strbuff))
                        i = 40;
                    else
                        i = atoi(strbuff);
                } else {
                    printf("                    Cylinders: (40, 80)  [80]: ");
                    fgets(strbuff, sizeof(strbuff), stdin);

                    if (!strlen(strbuff))
                        i = 80;
                    else
                        i = atoi(strbuff);
                }
            } while ((i != 40) && (i != 80));
            cyl = i;
        }
        else {
            if ((cyl != 40) && (cyl != 80)) {
                printf(" Illegal value for cylinders: %i\n", cyl);
                return 1;
            }
        }
    }
    sectors = (cyl * heads * spt);

    if (spc == -1) {
        do {
            printf("                     Sectors per Cluster [1]: ");
            fgets(strbuff, sizeof(strbuff), stdin);

            if (!strlen(strbuff))
                i = 1;
            else
                i = atoi(strbuff);
        } while (i > 255);
        spc = i;
    }
    else {
        if ((spc != 1) && (spc != 2) && (spc != 4) && (spc != 8) && (spc != 16)) {
            printf(" Illegal value for SPC: %i\n", spc);
            return 1;
        }
    }

    if (fats == -1) {
        do {
            printf("                          Number of FAT's [2]: ");
            fgets(strbuff, sizeof(strbuff), stdin);

            if (!strlen(strbuff))
                fats = 2;
            else
                fats = atoi(strbuff);
        } while ((fats < 1) || (fats > 2));
    }
    else {
        if ((fats != 1) && (fats != 2)) {
            printf(" Illegal value for FATs: %i\n", fats);
            return 1;
        }
    }

    if (fatz == -1) {
        do {
            printf("                               FAT Size: [12]: ");
            fgets(strbuff, sizeof(strbuff), stdin);

            if (!strlen(strbuff))
                i = 12;
            else
                i = atoi(strbuff);
        } while ((i != 12) && (i != 16) && (i != 32));
        fatz = i;
    }
    else {
        if ((fatz != 12) && (fatz != 16) && (fatz != 32)) {
            printf(" Illegal value for FAT size: %i\n", fatz);
            return 1;
        }
    }

    switch (fatz) {
    case 12:
        if ((sectors / (uint32_t)spc) > 4086L) {
            printf(" *** Illegal Size disk with FAT 12 *** \n");
            return 1;
        }
        spfat = (int)((int)((float)sectors * 1.5) / (512 * spc)) + 1;

        // actual count bytes on last fat sector needed as zeros (???)
        last_cnt = ((sectors - ((fats * spfat) + 17)) / spc);
        last_cnt = ((uint32_t)((float)last_cnt * 1.5) % 512);

        break;
    case 16:
        if ((sectors / (uint32_t)spc) > 65526UL) {
            printf(" *** Illegal Size disk with FAT 16 *** \n");
            return 1;
        }
        spfat = (int)((sectors << 1) / (512 * spc)) + 1;

        // actual count bytes on last fat sector needed as zeros (???)
        last_cnt = ((sectors - ((fats * spfat) + 17)) / spc);
        last_cnt = ((uint32_t)(last_cnt << 1) % 512);

        break;
    default:
        spfat = (int)((sectors << 2) / (512 * spc)) + 1;

        // actual count bytes on last fat sector needed as zeros (???)
        last_cnt = ((sectors - ((fats * spfat) + 17)) / spc);
        last_cnt = ((uint32_t)(last_cnt << 2) % 512);
    }

    if (verbose)
        printf("       Creating file:    %s\n"
            "           Cylinders:    %i\n"
            "               Sides:    %i\n"
            "       Sectors/Track:    %i\n"
            "       Total Sectors:    %lu\n"
            "                Size:    %3.2f (megs)\n"
            "     Sectors/Cluster:    %i\n"
            "               FAT's:    %i\n"
            "         Sectors/FAT:    %i\n"
            "            FAT size:    %i\n",
            filename, cyl, heads, spt, sectors,
            (float)((float)sectors / 2000.0), spc, fats, spfat, fatz);

    if (bootnamegiven) {
        if ((sfp = fopen(bootname, "rb")) == NULL) {
            printf("\nError opening file [%s]", bootname);
            return 1;
        }

        fseek(sfp, 0, SEEK_END);
        sec_res = ftell(sfp);
        fclose(sfp);

        if (sec_res % 512)
            sec_res = ((sec_res + 511) / 512);
        else
            sec_res /= 512;

        if (verbose)
            printf("\n Using Boot file %s of %i sectors", bootname, sec_res);
    }

    if (mbrfilegiven) {
        if ((mfp = fopen(mbrfile, "rb")) == NULL) {
            printf("\nError opening file [%s]", mbrfile);
            return 1;
        }

        fclose(mfp);

        if (verbose)
            printf("\n Using MBR file %s", mbrfile);
    }

    if ((fp = fopen(filename, "wb")) == NULL) {
        printf("\nError creating file [%s]", filename);
        return 1;
    }

    if (verbose)
        printf("\n\nWorking[");

    if (make_mbr) {
        // Make a simple MBR that points to the partition 63 sectors away.
        if (mbrfilegiven) {
            mfp = fopen(mbrfile, "rb");
            fseek(mfp, 0, SEEK_SET);
            fread(boot_sector, 1, 512, mfp);
            fclose(mfp);
        } else {
            memcpy(boot_sector, empty_mbr, 512);
        }

        struct PART_TBLE *part_tble = (struct PART_TBLE *) (boot_sector + 446);
        memset(part_tble, 0, 4 * sizeof(struct PART_TBLE));
        part_tble->bi = 0x80;
        part_tble->s_sector = 1;  // sectors are 1 based
        part_tble->s_head = 1;
        part_tble->s_cyl = 0;

        if (fatz == 12)
            part_tble->si = 0x01;
        else if (fatz == 16)
            part_tble->si = (sectors < 65536) ? 0x04 : 0x06;   // System ID
        else
            part_tble->si = 0x0B;

        if ((sectors - 1 + 63) <= 16450560UL) {  // 16450560 = 1024 cyl, 255 heads, 63 spt
            const uint8_t sects = (uint8_t)(((sectors - 1 + 63) % spt) + 1) & 0x3F;
            const uint16_t cyls = (uint16_t)(((sectors - 1 + 63) / spt) / heads);
            part_tble->e_sector = (uint8_t)(((cyl & 0x300) >> 2) | sects);
            part_tble->e_head = (uint8_t)(((sectors - 1 + 63) / spt) % heads);
            part_tble->e_cyl = (uint8_t)(cyls & 0xFF);
        } else {
            part_tble->e_sector = 0xFF;
            part_tble->e_head = 0xFF;
            part_tble->e_cyl = 0xFF;
        }
        part_tble->startlba = 63;
        part_tble->size = sectors;

        // Write the MBR and padding sectors
        if (verbose)
            putchar('.');

        write_sector(fp, boot_sector);
        memset(boot_sector, 0, 512);

        for (i = 1; i < 63; i++)
            write_sector(fp, boot_sector);
    }

    // Create BPB/boot block

    if (sec_res == -1) {
        memset(boot_sector, 0, 512);  // first, clear it out
    } else {
        sfp = fopen(bootname, "rb");
        fread(boot_sector, 512, 1, sfp);
    }

    switch (fatz) {
    case 12:
    case 16: {
        struct S_FAT1216_BPB *bpb = (struct S_FAT1216_BPB *) boot_sector;
        bpb->nBytesPerSec = 512;
        bpb->nSecPerClust = spc;
        bpb->nSecRes = (sec_res > 0) ? sec_res : 1;
        bpb->nFATs = fats;
        bpb->nRootEnts = (sectors >= 1440) ? 224 : 64;

        if (sectors < 65536) {
            bpb->nSecs = (uint16_t)sectors;
            bpb->nSecsExt = 0;
        } else {
            bpb->nSecs = 0;
            bpb->nSecsExt = sectors;
        }

        bpb->mDesc = media_descriptor(make_hd, heads, spt, cyl);
        bpb->nSecPerFat = spfat;
        bpb->nSecPerTrack = spt;
        bpb->nHeads = heads;
        bpb->nSecHidden = (make_mbr) ? 63 : 0;
        bpb->DriveNum = make_hd << 7;
        bpb->nResByte = 0;

        if (sec_res == -1) {
            bpb->jmps[0] = 0xEB; bpb->jmps[1] = 0x3C;
            bpb->nop = 0x90;
            memcpy(bpb->oemname, "MKFATIMG", 8);

            bpb->sig = 0x29;
            bpb->SerNum = 0;
            memcpy(bpb->VolName, "           ", 11);
            sprintf(strbuff, "FAT%2i   ", fatz);
            memcpy(bpb->FSType, strbuff, 8);

            memcpy(bpb->filler, boot_code, sizeof(boot_code));
            memcpy(bpb->filler + sizeof(boot_code), boot_data, sizeof(boot_data));
            memset(bpb->part_tble, 0, 4 * sizeof(struct PART_TBLE));

            if (make_hd && !make_mbr) {
                bpb->part_tble[0].bi = 0x80;
                bpb->part_tble[0].s_sector = 1;
                bpb->part_tble[0].si = (sectors < 65536) ? 0x04 : 0x06;
                // TODO: lba -> chs for ending CHS entries.
                bpb->part_tble[0].startlba = 0;
                bpb->part_tble[0].size = sectors;
            }

            bpb->boot_sig = 0xAA55;
        }

        bpbz = (void*)bpb->filler - (void*)bpb;
        break;
    }

    case 32: {
        struct S_FAT32_BPB *bpb = (struct S_FAT32_BPB *) boot_sector;
        bpb->nBytesPerSec = 512;
        bpb->nSecPerClust = spc;
        bpb->nSecRes = SECT_RES32;
        bpb->nFATs = fats;
        bpb->nRootEnts = 0;
        bpb->nSecs = 0;
        bpb->nSecsExt = sectors;
        bpb->mDesc = media_descriptor(make_hd, heads, spt, cyl);
        bpb->nSecPerFat = 0;
        bpb->nSecPerTrack = spt;
        bpb->nHeads = heads;
        bpb->nSecHidden = (make_mbr) ? 63 : 0;
        bpb->sect_per_fat32 = spfat;
        bpb->DriveNum = make_hd << 7;
        bpb->ext_flags = 0x00;
        bpb->fs_version = 0;
        bpb->root_base_cluster = 0x02;
        bpb->fs_info_sec = 1;
        bpb->backup_boot_sec = 6;
        bpb->nResByte = 0;

        if (sec_res == -1) {
            bpb->jmps[0] = 0xEB; bpb->jmps[1] = 0x3C;
            bpb->nop = 0x90;
            memcpy(bpb->oemname, "MKFATIMG", 8);
            bpb->sig = 0x29;
            bpb->SerNum = rand();
            memcpy(bpb->VolName, "           ", 11);     // Volume Label
            memcpy(bpb->FSType, "FAT32   ", 8);          // File system type
            memcpy(bpb->filler, boot_code, sizeof(boot_code));
            memcpy(bpb->filler + sizeof(boot_code), boot_data, sizeof(boot_data));
            memset(bpb->part_tble, 0, 4 * sizeof(struct PART_TBLE));

            if (make_hd && !make_mbr) {
                bpb->part_tble[0].bi = 0x80;
                bpb->part_tble[0].s_sector = 1;  // sectors are 1 based
                bpb->part_tble[0].si = (sectors < 65536) ? 0x04 : 0x06;   // System ID
                                                                          // TODO: lba -> chs for ending CHS entries.
                bpb->part_tble[0].startlba = 0;
                bpb->part_tble[0].size = sectors;
            }

            bpb->boot_sig = 0xAA55;
        }

        bpbz = (void*)bpb->filler - (void*)bpb;
        break;
    }
    }

    // Write the BPB
    putdot();

    if (sec_res == -1) {
        write_sector(fp, boot_sector);
        sectors--;
        sfp = NULL;
    } else {
        sfp = fopen(bootname, "rb");
        fread(buffer, 512, 1, sfp);
        memcpy(buffer, boot_sector, bpbz);
        write_sector(fp, buffer);
        sectors--;
        if ((fatz == 12) || (fatz == 16)) {
            while (--sec_res) {
                fread(buffer, 512, 1, sfp);
                write_sector(fp, buffer);
                sectors--;
            }
        }
    }

    // if fat32, write the info sector
    if (fatz == 32) {
        struct S_FAT32_FSINFO fsInfo;
        memset(&fsInfo, 0, 512);
        fsInfo.sig0 = 0x41615252;
        fsInfo.sig1 = 0x61417272;
        fsInfo.free_clust_fnt = /* 0xFFFFFFFF;*/ (sectors - spfat - ROOT_SECTS) / spc;  // a good approximation
        fsInfo.next_free_clust = 2;
        fsInfo.trail_sig = 0xAA550000;
        putdot();
        write_sector(fp, &fsInfo);  // LBA 1

                                    // if the bootfile is larger than 1 sector, skip over the
                                    // info sector part and copy the rest to this area, up to
                                    // LBA 6.
        sec_res -= 2; // subtract LBA 0 and LBA 1
        if (sec_res > 0) {
            r = 4;    // (for LBA 2 -> LBA 5)
            fseek(sfp, (2 * 512), SEEK_SET);

            while (sec_res--) {
                r--;
                fread(buffer, 512, 1, sfp);
                write_sector(fp, buffer);
            }

            memset(buffer, 0, 512);

            while (r--)
                write_sector(fp, buffer);
        } else {
            memset(buffer, 0, 512);
            buffer[510] = 0x55;
            buffer[511] = 0xAA;
            write_sector(fp, buffer);   // LBA 2

            memset(buffer, 0, 512);
            write_sector(fp, buffer);   // LBA 3
            write_sector(fp, buffer);   // LBA 4
            write_sector(fp, buffer);   // LBA 5
        }

        write_sector(fp, boot_sector);  // LBA 6
        write_sector(fp, &fsInfo);  // LBA 7
        buffer[510] = 0x55;
        buffer[511] = 0xAA;
        write_sector(fp, buffer);   // LBA 8

        memset(buffer, 0, 512);

        for (r = 9; r<SECT_RES32; r++)
            write_sector(fp, buffer);

        sectors -= (SECT_RES32 - 1);  // - 1 = original boot sector
    }

    if (sfp)
        fclose(sfp);

    // Write the FAT(s)
    for (i = 0; i<fats; i++) {
        memset(buffer, 0, 512);
        switch (fatz) {
        case 32:
            buffer[7] = 0x0F;
            buffer[6] = 0xFF;
            buffer[5] = 0xFF;
            buffer[4] = 0xFF;
            buffer[3] = 0x0F;
            buffer[2] = 0xFF;
            buffer[1] = 0xFF;
            buffer[0] = media_descriptor(make_hd, heads, spt, cyl);
        case 16:
            buffer[3] = 0xFF;
            buffer[2] = 0xFF;
            buffer[1] = 0xFF;
            buffer[0] = media_descriptor(make_hd, heads, spt, cyl);
            break;
        case 12:
            buffer[2] = 0xFF;
            buffer[1] = 0xFF;
            buffer[0] = media_descriptor(make_hd, heads, spt, cyl);
            break;
        }

        putdot();
        write_sector(fp, &buffer);
        sectors--;
        memset(buffer, 0, 512);
        for (j = 0; j<spfat - 1; j++) {
            putdot();
            write_sector(fp, &buffer);
            sectors--;
        }
    }

    // Write the root
    memset(buffer, 0, 512);
    for (i = 0; i<ROOT_SECTS; i++) {
        putdot();
        write_sector(fp, &buffer);
        sectors--;
    }

    // Write data area
    memset(buffer, 0, 512);
    while (sectors--) {
        putdot();
        write_sector(fp, &buffer);
    }

    if (verbose)
        printf("]Done");

    // Close the file
    fclose(fp);

    return 0;
}

/*
Byte   Capacity   Media Size and Type
F0     2.88 MB     3.5-inch, 2-sided, 36-sector
F0     1.44 MB     3.5-inch, 2-sided, 18-sector
F9      720 KB     3.5-inch, 2-sided,  9-sector
F9      1.2 MB    5.25-inch, 2-sided, 15-sector
FD      360 KB    5.25-inch, 2-sided,  9-sector
FF      320 KB    5.25-inch, 2-sided,  8-sector
FC      180 KB    5.25-inch, 1-sided,  9-sector
FE      160 KB    5.25-inch, 1-sided,  8-sector
F8     -----      Fixed disk
*/
uint8_t media_descriptor(const bool ishd, const int heads, const int spt, const int cylinders) {
    if (ishd)
        return 0xF8;

    if ((heads == 2) && ((spt == 9) || (spt == 15)) && (cylinders == 80))
        return 0xF9;

    if ((heads == 2) && (spt == 9) && (cylinders == 40))
        return 0xFD;

    if ((heads == 2) && (spt == 8) && (cylinders == 40))
        return 0xFF;

    if ((heads == 1) && (spt == 9) && (cylinders == 40))
        return 0xFC;

    if ((heads == 1) && (spt == 8) && (cylinders == 40))
        return 0xFE;

    return 0xF0;
}

// Write sector(s)
void write_sector(FILE *fp, void *ptr) {
    if (fwrite(ptr, 512, 1, fp) < 1) {
        printf("\n **** Error writing to file ****");
        exit(-1);
    }
}

// Put a dot
void putdot() {
    if (verbose)
        if (!(sectors & 0x000003FFUL))
            putchar('.');
}

int get_params(int argc, char *argv[]) {
    int i;

    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-help")) {
            printf("Make disk image\n");
            printf(" Help Screen:\n"
                "    -help      prints this info\n"
                "    -noverb    don't print unnecassary items\n"
                "    -hd        hard drive image\n"
                "    -fd        floppy image\n"
                "    -no-mbr    don't add a Master Boot Record\n"
                "    -mbr       add the Master Boot Record\n"
                "    -z0000     if -hd used, this is the size in megabytes\n"
                "    -spt00     if -fd used, this is the sectors per track\n"
                "    -heads00   if -fd used, this is the number of heads\n"
                "    -cyls00    if -fd used, this is the number of cylinders\n"
                "    -spc00     this is the number of sectors per cluster\n"
                "    -fats0     number of fats to use (1 or 2)\n"
                "    -fatz00    fatsize, 12, 16, or 32\n"
                "    -b file    the file containing the bootsector to place in the first sector of the first partition\n"
                "    -m file    the file containing the bootsector to place at LBA 0 if -mbr is enabled\n"
                "    -l label   volume label\n"
                "    filename   the filename to create\n");
            return 1;
        } else if (!strcmp(argv[i], "-noverb")) {
            verbose = false;
            continue;
        } else if (!strcmp(argv[i], "-hd")) {
            make_hd = true;
            make_fd = false;
            continue;
        } else if (!strcmp(argv[i], "-fd")) {
            make_fd = true;
            make_hd = false;
            continue;
        } else if (!strcmp(argv[i], "-no-mbr")) {
            make_mbr = false;
            make_mbr_given = true;
            continue;
        } else if (!strcmp(argv[i], "-mbr")) {
            make_mbr = true;
            make_mbr_given = true;
            continue;
        } else if (!memcmp(argv[i], "-z", 2) && isdigit(argv[i][2])) {
            image_size = atoi(&argv[i][2]);
            continue;
        } else if (!memcmp(argv[i], "-spt", 4) && isdigit(argv[i][4])) {
            spt = atoi(&argv[i][4]);
            continue;
        } else if (!memcmp(argv[i], "-heads", 6) && isdigit(argv[i][6])) {
            heads = atoi(&argv[i][6]);
            continue;
        } else if (!memcmp(argv[i], "-cyls", 5) && isdigit(argv[i][5])) {
            cyl = atoi(&argv[i][5]);
            continue;
        } else if (!memcmp(argv[i], "-spc", 4) && isdigit(argv[i][4])) {
            spc = atoi(&argv[i][4]);
            continue;
        } else if (!memcmp(argv[i], "-fats", 5) && isdigit(argv[i][5])) {
            fats = atoi(&argv[i][5]);
            continue;
        } else if (!memcmp(argv[i], "-fatz", 5) && isdigit(argv[i][5])) {
            fatz = atoi(&argv[i][5]);
            continue;
        } else if (!memcmp(argv[i], "-b", 2)) {
            if (strlen(argv[i]) > 2) {
                strcpy(bootname, &argv[i][2]);
                bootnamegiven = true;
            } else {
                if ((i + 1) < argc) {
                    strcpy(bootname, argv[++i]);
                    bootnamegiven = true;
                } else {
                    printf("\n No filepath given for -b...");
                    return 1;
                }
            }
            continue;
        } else if (!memcmp(argv[i], "-m", 2)) {
            if (strlen(argv[i]) > 2) {
                strcpy(mbrfile, &argv[i][2]);
                mbrfilegiven = true;
            } else {
                if ((i + 1) < argc) {
                    strcpy(mbrfile, argv[++i]);
                    mbrfilegiven = true;
                } else {
                    printf("\n No filepath given for -m...");
                    return 1;
                }
            }
            continue;
        } else if (!memcmp(argv[i], "-l", 2)) {
            if (strlen(argv[i]) > 2) {
                strcpy(volumelabel, &argv[i][2]);
                volumelabelgiven = true;
            } else {
                if ((i + 1) < argc) {
                    strcpy(volumelabel, argv[++i]);
                    volumelabelgiven = true;
                } else {
                    printf("\n No volume label given...");
                    return 1;
                }
            }
            continue;
        } else if (argv[i][0] == '-') {
            printf("\n Unknown Parameter: '%s'", argv[i]);
            return 1;
        }

        // else, it must be the filename
        strcpy(filename, argv[i]);
        filenamegiven = true;
    }

    return 0;
}
