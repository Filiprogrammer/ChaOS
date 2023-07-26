#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "fat.h"

uint8_t partition = 1;
char* imagefile = NULL;
char* fatfile = NULL;
char* filetocopy = NULL;
char* volumelabel = NULL;
char* fatdir = NULL;

char *strrpbrk(const char *szString, const char *szChars) {
    const char  *p;
    const char *p1;
    const char *p0;

    for (p = szChars, p0 = p1 = NULL; p && *p; ++p) {
        p1 = strrchr(szString, *p);

        if (p1 && p1 > p0)
            p0 = p1;
    }
    return (char*) p0;
}

int get_params(int args, char* argv[]);

int main(int argc, char* argv[]) {
    get_params(argc, argv);

    if (imagefile == NULL) {
        puts("No image file given");
        exit(1);
    }

    if (volumelabel == NULL && fatdir == NULL) {
        if (filetocopy == NULL) {
            puts("No filetocopy given");
            exit(1);
        }

        if (fatfile == NULL) {
            fatfile = strrpbrk(filetocopy, "/\\");

            if (fatfile == NULL)
                fatfile = filetocopy;
        }
    }

    storage_devManager_init(imagefile);

    uint8_t bootsector[512];
    storage_readSector(NULL, 0, bootsector, 512);

    uint32_t lbaStart = 0;
    uint32_t lbaLength = storage_size() / 512;

    if (strncmp((char*)&bootsector + 0x36, "FAT12   ", 8) != 0 &&
        strncmp((char*)&bootsector + 0x36, "FAT16   ", 8) != 0 &&
        strncmp((char*)&bootsector + 0x52, "FAT32   ", 8) != 0)
    {
        lbaStart = *((uint32_t*)(bootsector + 0x1AE + partition * 16 + 0x08));
        lbaLength = *((uint32_t*)(bootsector + 0x1AE + partition * 16 + 0x0C));
    }

    FAT* fat = FAT_create(NULL, lbaStart, lbaLength, FAT12);

    if (volumelabel == NULL) {
        if (fatdir == NULL) {
            FILE* fp = fopen(filetocopy, "rb");
            if (fp == NULL) {
                fprintf(stderr, "cannot open file %s\n", filetocopy);
                exit(1);
            }

            fseek(fp, 0L, SEEK_END);
            uint32_t sz = ftell(fp);
            fseek(fp, 0L, SEEK_SET);
            uint8_t* contents = (uint8_t*)malloc(sz);
            fread(contents, 1, sz, fp);
            fclose(fp);

            file_t file = { { 0 },{ 0 }, 0, 0, NULL };
            FAT_abstract_createFile(fat, fatfile, &file);
            FAT_writeFileContents(fat, fatfile, contents, sz);
        } else {
            FAT_abstract_createDirectory(fat, fatdir);
        }
    } else {
        FAT_setVolumeLabel(fat, volumelabel);
    }

    storage_bye();
    FAT_destroy(fat);
    exit(0);

    return 0;
}

int get_params(int argc, char *argv[]) {
    int i;

    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-help")) {
            printf(" Help Screen:\n"
                "    -help           prints this info\n"
                "    -i imagefile    the image file\n"
                "    -l volumelabel  the new volume label\n"
                "    -d directory    the new directory to create\n"
                "    -f fatfile      the target file on the image\n"
                "    -p partition    the number of the partition (1 - 4)\n"
                "    filetocopy      the file to copy\n");
            exit(1);
        } else if (!memcmp(argv[i], "-p", 2) && isdigit(argv[i][2])) {
            partition = atoi(&argv[i][2]);
            if (partition < 1 && partition > 4) {
                printf("Invalid partition %d. Only 1 - 4 would be valid", partition);
                exit(1);
            }
            continue;
        } else if (!memcmp(argv[i], "-i", 2)) {
            if (strlen(argv[i]) > 2) {
                uint32_t len = strlen(&argv[i][2]);
                imagefile = (char*) malloc(len + 1);
                strcpy(imagefile, &argv[i][2]);
                imagefile[len] = 0;
            } else {
                if ((i + 1) < argc) {
                    uint32_t len = strlen(argv[++i]);
                    imagefile = (char*)malloc(len + 1);
                    strcpy(imagefile, argv[i]);
                    imagefile[len] = 0;
                } else {
                    printf("\n No imagefile given...");
                    return 1;
                }
            }
            continue;
        } else if (!memcmp(argv[i], "-f", 2)) {
            if (strlen(argv[i]) > 2) {
                uint32_t len = strlen(&argv[i][2]);
                fatfile = (char*)malloc(len + 1);
                strcpy(fatfile, &argv[i][2]);
                fatfile[len] = 0;
            } else {
                if ((i + 1) < argc) {
                    uint32_t len = strlen(argv[++i]);
                    fatfile = (char*)malloc(len + 1);
                    strcpy(fatfile, argv[i]);
                    fatfile[len] = 0;
                } else {
                    printf("\n No file on the image given...");
                    return 1;
                }
            }
            continue;
        } else if (!memcmp(argv[i], "-l", 2)) {
            if (strlen(argv[i]) > 2) {
                uint32_t len = strlen(&argv[i][2]);
                volumelabel = (char*)malloc(len + 1);
                strcpy(volumelabel, &argv[i][2]);
                volumelabel[len] = 0;
            } else {
                if ((i + 1) < argc) {
                    uint32_t len = strlen(argv[++i]);
                    volumelabel = (char*)malloc(len + 1);
                    strcpy(volumelabel, argv[i]);
                    volumelabel[len] = 0;
                }
            }
            continue;
        } else if (!memcmp(argv[i], "-d", 2)) {
            if (strlen(argv[i]) > 2) {
                uint32_t len = strlen(&argv[i][2]);
                fatdir = (char*)malloc(len + 1);
                strcpy(fatdir, &argv[i][2]);
                fatdir[len] = 0;
            } else {
                if ((i + 1) < argc) {
                    uint32_t len = strlen(argv[++i]);
                    fatdir = (char*)malloc(len + 1);
                    strcpy(fatdir, argv[i]);
                    fatdir[len] = 0;
                }
            }
            continue;
        } else if (argv[i][0] == '-') {
            printf("\n Unknown Parameter: '%s'", argv[i]);
            exit(1);
        }

        // else, it must be the filename
        uint32_t len = strlen(argv[i]);
        filetocopy = (char*)malloc(len + 1);
        strcpy(filetocopy, argv[i]);
        filetocopy[len] = 0;
    }

    return 0;
}
