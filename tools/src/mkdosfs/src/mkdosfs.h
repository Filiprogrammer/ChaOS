#include <stdint.h>

struct PART_TBLE {
    uint8_t  bi;
    uint8_t  s_head;    // 8 bit head count
    uint8_t  s_sector;  // hi 2 bits is hi 2 bits of cyl, bottom 6 bits is sector
    uint8_t  s_cyl;     // bottom 8 bits
    uint8_t  si;
    uint8_t  e_head;    // 8 bit head count
    uint8_t  e_sector;  // hi 2 bits is hi 2 bits of cyl, bottom 6 bits is sector
    uint8_t  e_cyl;     // bottom 8 bits
    uint32_t startlba;
    uint32_t size;
} __attribute__((packed));

struct S_FAT1216_BPB {
    uint8_t  jmps[2];       // The jump short instruction
    uint8_t  nop;           // nop instruction;
    char     oemname[8];    // OEM name
    uint16_t nBytesPerSec;  // Bytes per sector
    uint8_t  nSecPerClust;  // Sectors per cluster
    uint16_t nSecRes;       // Sectors reserved for Boot Record
    uint8_t  nFATs;         // Number of FATs
    uint16_t nRootEnts;     // Max Root Directory Entries allowed
    uint16_t nSecs;         // Number of Logical Sectors (0B40h)
    uint8_t  mDesc;         // Medium Descriptor Byte
    uint16_t nSecPerFat;    // Sectors per FAT
    uint16_t nSecPerTrack;  // Sectors per Track
    uint16_t nHeads;        // Number of Heads
    uint32_t nSecHidden;    // Number of Hidden Sectors
    uint32_t nSecsExt;      // This value used when there are more
    uint8_t  DriveNum;      // Physical drive number
    uint8_t  nResByte;      // Reserved (we use for FAT type (12- 16-bit)
    uint8_t  sig;           // Signature for Extended Boot Record
    uint32_t SerNum;        // Volume Serial Number
    char     VolName[11];   // Volume Label
    char     FSType[8];     // File system type
    uint8_t  filler[384];
    struct PART_TBLE part_tble[4]; // partition table
    uint16_t boot_sig;
} __attribute__((packed));

#define  SECT_RES32  32   // sectors reserved

struct S_FAT32_BPB {
    uint8_t  jmps[2];
    uint8_t  nop;           // nop instruction;
    char     oemname[8];
    uint16_t nBytesPerSec;
    uint8_t  nSecPerClust;
    uint16_t nSecRes;
    uint8_t  nFATs;
    uint16_t nRootEnts;
    uint16_t nSecs;
    uint8_t  mDesc;
    uint16_t nSecPerFat;
    uint16_t nSecPerTrack;
    uint16_t nHeads;
    uint32_t nSecHidden;
    uint32_t nSecsExt;
    uint32_t sect_per_fat32; // offset 36 (24h)
    uint16_t ext_flags;      // bit 8 = write to all copies of FAT(s).  bit0:3 = which fat is active
    uint16_t fs_version;
    uint32_t root_base_cluster; //
    uint16_t fs_info_sec;
    uint16_t backup_boot_sec;
    uint8_t  reserved[12];
    uint8_t  DriveNum;  // not FAT specific
    uint8_t  nResByte;
    uint8_t  sig;
    uint32_t SerNum;
    char     VolName[11];
    char     FSType[8];
    uint8_t  filler[356];
    struct PART_TBLE part_tble[4]; // partition table
    uint16_t boot_sig;
} __attribute__((packed));

struct S_FAT32_FSINFO {
    uint32_t sig0;              // 0x41615252 ("RRaA")
    uint8_t  resv[480];
    uint32_t sig1;              // 0x61417272 ("rrAa")
    uint32_t free_clust_fnt;    // 0xFFFFFFFF when the count is unknown
    uint32_t next_free_clust;   // most recent allocated cluster  + 1
    uint8_t  resv1[12];
    uint32_t trail_sig;         // 0xAA550000
} __attribute__((packed));


unsigned char boot_code[] = {
    0xFA,           //CLI
    0xB8,0xC0,0x07, //MOV AX,07C0
    0x8E,0xD8,      //MOV DS,AX
    0x8E,0xD0,      //MOV SS,AX
    0xBC,0x00,0x40, //MOV SP,4000
    0xFB,           //STI
    0xBE,0x6B,0x00, //MOV SI,006B
    0xE8,0x06,0x00, //CALL  0156
    0x30,0xE4,      //XOR AH,AH
    0xCD,0x16,      //INT 16
    0xCD,0x18,      //INT 18
    0x50,           //PUSH  AX
    0x53,           //PUSH  BX
    0x56,           //PUSH  SI
    0xB4,0x0E,      //MOV AH,0E
    0x31,0xDB,      //XOR BX,BX
    0xFC,           //CLD
    0xAC,           //LODSB
    0x08,0xC0,      //OR  AL,AL
    0x74,0x04,      //JZ  0167
    0xCD,0x10,      //INT 10
    0xEB,0xF6,      //JMP 015D
    0x5E,           //POP SI
    0x5B,           //POP BX
    0x58,           //POP AX
    0xC3,           //RET
    13,10
};
unsigned char boot_data[] = "Error reading disk or Non-System Disk"
"\x0D\x0A"
"Press a key to reboot\x00";

unsigned char empty_mbr[] = {
    0xFA, 0xB8, 0xC0, 0x07, 0x8E, 0xD8, 0x8E, 0xD0, 0xBC, 0x00, 0x40, 0xFB, 0xBE, 0x24, 0x00, 0xE8,
    0x03, 0x00, 0xF4, 0xEB, 0xFD, 0xB4, 0x0E, 0x31, 0xDB, 0xFC, 0xAC, 0x08, 0xC0, 0x74, 0x04, 0xCD,
    0x10, 0xEB, 0xF6, 0xC3, 0x03, 0x0A, 0x07, 0x49, 0x20, 0x61, 0x6D, 0x20, 0x61, 0x6E, 0x20, 0x65,
    0x6D, 0x70, 0x74, 0x79, 0x20, 0x62, 0x6F, 0x6F, 0x74, 0x20, 0x73, 0x65, 0x63, 0x74, 0x6F, 0x72,
    0x2E, 0x20, 0x20, 0x49, 0x20, 0x77, 0x69, 0x6C, 0x6C, 0x20, 0x6A, 0x75, 0x73, 0x74, 0x20, 0x68,
    0x61, 0x6C, 0x74, 0x20, 0x68, 0x65, 0x72, 0x65, 0x2E, 0x00, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x55, 0xAA
};
