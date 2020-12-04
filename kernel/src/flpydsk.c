#include "flpydsk.h"
#include "math.h"

// IO ports
enum FLPYDSK_IO {
    FLPYDSK_DOR  = 0x3f2,
    FLPYDSK_MSR  = 0x3f4,
    FLPYDSK_FIFO = 0x3f5,
    FLPYDSK_CTRL = 0x3f7
};

// Bits 0-4 of command byte.
enum FLPYDSK_CMD {
    FDC_CMD_READ_TRACK   =    2,
    FDC_CMD_SPECIFY      =    3,
    FDC_CMD_CHECK_STAT   =    4,
    FDC_CMD_WRITE_SECT   =    5,
    FDC_CMD_READ_SECT    =    6,
    FDC_CMD_CALIBRATE    =    7,
    FDC_CMD_CHECK_INT    =    8,
    FDC_CMD_FORMAT_TRACK =   13,
    FDC_CMD_SEEK         =   15,
    FDC_CMD_VERSION      =   16
};

// Additional command masks. Can be masked with above commands
enum FLPYDSK_CMD_EXT {
    FDC_CMD_EXT_SKIP       = 0x20,
    FDC_CMD_EXT_DENSITY    = 0x40,
    FDC_CMD_EXT_MULTITRACK = 0x80
};

// Digital Output Register
enum FLPYDSK_DOR_MASK {
    FLPYDSK_DOR_MASK_DRIVE0         =    0,
    FLPYDSK_DOR_MASK_DRIVE1         =    1,
    FLPYDSK_DOR_MASK_DRIVE2         =    2,
    FLPYDSK_DOR_MASK_DRIVE3         =    3,
    FLPYDSK_DOR_MASK_RESET          = 0x04,
    FLPYDSK_DOR_MASK_DMA            = 0x08,
    FLPYDSK_DOR_MASK_DRIVE0_MOTOR   = 0x10,
    FLPYDSK_DOR_MASK_DRIVE1_MOTOR   = 0x20,
    FLPYDSK_DOR_MASK_DRIVE2_MOTOR   = 0x40,
    FLPYDSK_DOR_MASK_DRIVE3_MOTOR   = 0x80
};

// Main Status Register
enum FLPYDSK_MSR_MASK {
    FLPYDSK_MSR_MASK_DRIVE1_POS_MODE    = 0x01,
    FLPYDSK_MSR_MASK_DRIVE2_POS_MODE    = 0x02,
    FLPYDSK_MSR_MASK_DRIVE3_POS_MODE    = 0x04,
    FLPYDSK_MSR_MASK_DRIVE4_POS_MODE    = 0x08,
    FLPYDSK_MSR_MASK_BUSY               = 0x10,
    FLPYDSK_MSR_MASK_DMA                = 0x20,
    FLPYDSK_MSR_MASK_DATAIO             = 0x40,
    FLPYDSK_MSR_MASK_DATAREG            = 0x80
};

// Controller Status Port 0
enum FLPYDSK_ST0_MASK {
    FLPYDSK_ST0_MASK_DRIVE0     = 0,
    FLPYDSK_ST0_MASK_DRIVE1     = 1,
    FLPYDSK_ST0_MASK_DRIVE2     = 2,
    FLPYDSK_ST0_MASK_DRIVE3     = 3,
    FLPYDSK_ST0_MASK_HEADACTIVE = 0x04,
    FLPYDSK_ST0_MASK_NOTREADY   = 0x08,
    FLPYDSK_ST0_MASK_UNITCHECK  = 0x10,
    FLPYDSK_ST0_MASK_SEEKEND    = 0x20,
    FLPYDSK_ST0_MASK_INTCODE    = 0x40
};

// FLPYDSK_ST0_MASK_INTCODE types
enum FLPYDSK_ST0_INTCODE_TYP {
    FLPYDSK_ST0_TYP_NORMAL       = 0,
    FLPYDSK_ST0_TYP_ABNORMAL_ERR = 1,
    FLPYDSK_ST0_TYP_INVALID_ERR  = 2,
    FLPYDSK_ST0_TYP_NOTREADY     = 3
};

// GAP 3 sizes
enum FLPYDSK_GAP3_LENGTH {
    FLPYDSK_GAP3_LENGTH_STD  = 42,
    FLPYDSK_GAP3_LENGTH_5_14 = 32,
    FLPYDSK_GAP3_LENGTH_3_5  = 27
};

// Formula: 2^sector_number * 128
enum FLPYDSK_SECTOR_DTL {
    FLPYDSK_SECTOR_DTL_128  = 0,
    FLPYDSK_SECTOR_DTL_256  = 1,
    FLPYDSK_SECTOR_DTL_512  = 2,
    FLPYDSK_SECTOR_DTL_1024 = 4
};

typedef enum {
    floppy_dir_read = 1,
    floppy_dir_write = 2
} floppy_dir;

#define FLPYDSK_MOTOR_DELAY_MS 300
#define FLPY_SECTORS_PER_TRACK 18
//#define FLPY_MOTOR_TURN_OFF_SECONDS 20

bool _irqFired = false;
uint8_t dor;
//int16_t motorTurnOffTimer = 0;
uint8_t lastAccessedDrive = 0;
uint32_t lastAccessedSector = 0;
uint32_t lastAccessedSize = 0;
bool useLastAccessed = false;

bool flpy_sendCommand(uint8_t cmd) {
    for (uint8_t i = 0; i < 100; ++i) {
        if (FLPYDSK_MSR_MASK_DATAREG & inportb(FLPYDSK_MSR)) {
            outportb(FLPYDSK_FIFO, cmd);
            return true;
        }
    }

    return false;  // Timeout
}

uint8_t flpy_readData() {
    for (uint8_t i = 0; i < 100; ++i) {
        if (FLPYDSK_MSR_MASK_DATAREG & inportb(FLPYDSK_MSR)) {
            return inportb(FLPYDSK_FIFO);
        }
    }

    return 0;  // Timeout
}

void flpy_checkInterrupt(uint8_t* st0, uint8_t* cyl) {
    flpy_sendCommand(FDC_CMD_CHECK_INT);
    *st0 = flpy_readData();
    *cyl = flpy_readData();
}

void flpy_writeDOR(uint8_t val) {
    dor = val;
    outportb(FLPYDSK_DOR, val);
}

bool flpy_controlMotor(uint8_t drive, bool enable) {
    if (drive > 3) return false;

    if (enable) {
        if (!(dor & (FLPYDSK_DOR_MASK_DRIVE0_MOTOR << drive))) {
            dor |= FLPYDSK_DOR_MASK_DRIVE0_MOTOR << drive;
            flpy_writeDOR(dor);
            sleepMilliSeconds(FLPYDSK_MOTOR_DELAY_MS);
        }
    } else {
        dor &= ~(FLPYDSK_DOR_MASK_DRIVE0_MOTOR << drive);
        flpy_writeDOR(dor);
        // TODO: make it so that you don't have to wait for the motor to turn off (probably motor_state = turning_off)
        sleepMilliSeconds(FLPYDSK_MOTOR_DELAY_MS);
    }

    return true;
}

bool flpy_irqWait() {
    sti();
    uint32_t timeout = timer_getMilliseconds() + 2000;

    while (timer_getMilliseconds() < timeout) {
        if (_irqFired) {
            _irqFired = false;
            return true;  // IRQ Fired
        }

        hlt();
    }

    return false;  // Timeout
}

void flpy_lbaToChs(uint32_t lba, uint8_t* cyl, uint8_t* head, uint8_t* sector) {
    *cyl    =  lba / (FLPY_SECTORS_PER_TRACK * 2);
    *head   = (lba % (FLPY_SECTORS_PER_TRACK * 2)) / FLPY_SECTORS_PER_TRACK;
    *sector = (lba % (FLPY_SECTORS_PER_TRACK * 2)) % FLPY_SECTORS_PER_TRACK + 1;
}

bool flpy_calibrate(uint8_t drive) {
    if (drive > 3) return false;

    flpy_controlMotor(drive, true);

    uint8_t st0, cyl;

    for (uint8_t i = 0; i < 10; ++i) {
        flpy_sendCommand(FDC_CMD_CALIBRATE);
        flpy_sendCommand(drive);

        flpy_irqWait();

        flpy_checkInterrupt(&st0, &cyl);

        if (!(st0 & FLPYDSK_ST0_MASK_SEEKEND))
            continue;

        if (!cyl) {
            // found cylinder 0
            //flpy_controlMotor(drive, false);
            return true;
        }
    }

    //flpy_controlMotor(drive, false);
    return false;
}

bool flpy_seek(uint8_t drive, uint8_t cyli, uint16_t head) {
    if (drive > 3) return false;

    flpy_controlMotor(drive, true);

    uint8_t st0, cyl;

    for (uint8_t i = 0; i < 10; ++i) {
        flpy_sendCommand(FDC_CMD_SEEK);
        flpy_sendCommand((head << 2) | drive);
        flpy_sendCommand(cyli);

        flpy_irqWait();

        flpy_checkInterrupt(&st0, &cyl);

        if (!(st0 & FLPYDSK_ST0_MASK_SEEKEND))
            continue;

        if (cyl == cyli) {
            //flpy_controlMotor(drive, false);
            return true;
        }
    }

    //flpy_controlMotor(drive, false);
    return false;
}

bool flpy_dmaInit(floppy_dir dir, uint32_t addr, uint32_t len) {
    if (addr >> 24) return false;
    len -= 1;
    if (len >> 16) return false;
    uint8_t mode;
    switch (dir) {
        case floppy_dir_read:
            mode = 0x46;
            break;
        case floppy_dir_write:
            mode = 0x4a;
            break;
        default:
            return false;
    }

    outportb(0x0a, 0x06);         // mask chan 2

    outportb(0x0c, 0xff);         // reset flip-flop
    outportb(0x04, addr);         //  - address low byte
    outportb(0x04, addr >> 8);    //  - address high byte

    outportb(0x81, addr >> 16);   // external page register

    outportb(0x0c, 0xff);         // reset flip-flop
    outportb(0x05, len);          //  - count low byte
    outportb(0x05, len >> 8);     //  - count high byte

    outportb(0x0b, mode);         // set mode (see above)

    outportb(0x0a, 0x02);         // unmask chan 2
    return true;
}

bool flpy_transfer_sectors(uint8_t drive, uint8_t head, uint8_t cyl, uint8_t sector, uint8_t numberOfSectors, floppy_dir dir) {
    if (drive > 3) return false;
    if (numberOfSectors > ((size_t)FLPY_DMA_BUFFER_LEN / 512)) return false;

    uint8_t cmd = FDC_CMD_EXT_MULTITRACK | FDC_CMD_EXT_DENSITY;

    switch (dir) {
        case floppy_dir_read:
            cmd |= FDC_CMD_READ_SECT;
            break;
        case floppy_dir_write:
            cmd |= FDC_CMD_WRITE_SECT;
            break;
        default:
            return false;
    }

    // seek both heads
    if (!flpy_seek(drive, cyl, 0)) return false;
    if (!flpy_seek(drive, cyl, 1)) return false;

    flpy_controlMotor(drive, true);

    for (uint8_t i = 0; i < 20; ++i) {
        if (!flpy_dmaInit(dir, FLPY_DMA_BUFFER, numberOfSectors * 512))
            return false;

        flpy_sendCommand(cmd);
        flpy_sendCommand(head << 2 | drive);
        flpy_sendCommand(cyl);
        flpy_sendCommand(head);
        flpy_sendCommand(sector);
        flpy_sendCommand(FLPYDSK_SECTOR_DTL_512);        // bytes per sector (128 * x^2)
        flpy_sendCommand(sector + numberOfSectors - 1);  // last sector to transfer
        flpy_sendCommand(FLPYDSK_GAP3_LENGTH_3_5);       // default gap length for 3.5"
        flpy_sendCommand(0xFF);

        flpy_irqWait();

        uint8_t st0, st1, st2, bps;
        st0 = flpy_readData();
        st1 = flpy_readData();
        st2 = flpy_readData();
        flpy_readData();
        flpy_readData();
        flpy_readData();
        bps = flpy_readData();

        uint8_t error = 0;

        if (st0 & 0xC0) {
            static const char* status[] =
                {0, "error", "invalid command", "drive not ready"};
            printf("flpy_transfer_sectors: status = %s\n", status[st0 >> 6]);
            error = 1;
        }
        if (st1 & 0x80) {
            puts("flpy_transfer_sectors: end of cylinder\n");
            error = 1;
        }
        if (st0 & 0x08) {
            puts("flpy_transfer_sectors: drive not ready\n");
            error = 1;
        }
        if (st1 & 0x20) {
            puts("flpy_transfer_sectors: CRC error\n");
            error = 1;
        }
        if (st1 & 0x10) {
            puts("flpy_transfer_sectors: controller timeout\n");
            error = 1;
        }
        if (st1 & 0x04) {
            puts("flpy_transfer_sectors: no data found\n");
            error = 1;
        }
        if ((st1 | st2) & 0x01) {
            puts("flpy_transfer_sectors: no address mark found\n");
            error = 1;
        }
        if (st2 & 0x40) {
            puts("flpy_transfer_sectors: deleted address mark\n");
            error = 1;
        }
        if (st2 & 0x20) {
            puts("flpy_transfer_sectors: CRC error in data\n");
            error = 1;
        }
        if (st2 & 0x10) {
            puts("flpy_transfer_sectors: wrong cylinder\n");
            error = 1;
        }
        if (st2 & 0x04) {
            puts("flpy_transfer_sectors: uPD765 sector not found\n");
            error = 1;
        }
        if (st2 & 0x02) {
            puts("flpy_transfer_sectors: bad cylinder\n");
            error = 1;
        }
        if (bps != 0x2) {
            printf("flpy_transfer_sectors: wanted 512B/sector, got %d", (1 << (bps + 7)));
            error = 1;
        }
        if (st1 & 0x02) {
            puts("flpy_transfer_sectors: not writable\n");
            error = 2;
        }

        if (!error) {
            //flpy_controlMotor(drive, false);
            return true;
        }
        if (error > 1) {
            puts("flpy_transfer_sectors: not retrying..\n");
            //flpy_controlMotor(drive, false);
            return false;
        }
    }

    puts("flpy_transfer_sectors: 20 retries exhausted\n");
    //flpy_controlMotor(drive, false);
    return false;
}

/**
 * @brief Reads bytes from the floppy disk.
 * 
 * @param inst a pointer to the instance of a floppy disk
 * @param sectorLBA first sector to read from
 * @param addr buffer to write the data to
 * @param len number of bytes to read
 * @return true the read operation was successful.
 * @return false the read operation failed.
 */
bool flpydsk_read_sectors(Flpydsk_t* inst, uint32_t sectorLBA, uint8_t* addr, size_t len) {
    if (inst == NULL) return false;
    if (inst->drive > 3) return false;
    if (len == 0) return true;

    uint8_t head = 0, cyl = 0, sector = 1;
    uint32_t curSecLBA = sectorLBA;

    if (useLastAccessed && (inst->drive == lastAccessedDrive)) {
        if (curSecLBA >= lastAccessedSector && (curSecLBA < (lastAccessedSector + lastAccessedSize))) {
            //printf("curSecLBA: %u\tlen/512:%u\tlastAccessedSector: %u\tlastAccessedSize: %u\n", curSecLBA, len / 512, lastAccessedSector, lastAccessedSize);
            uint32_t numberOfSectors = MIN(curSecLBA + len / 512, lastAccessedSector + lastAccessedSize) - curSecLBA;
            //printf("numberOfSectors: %u", numberOfSectors);
            memcpy(addr, (void*)FLPY_DMA_BUFFER + (curSecLBA - lastAccessedSector), numberOfSectors * 512);
            curSecLBA += numberOfSectors;
        }
    }

    while (curSecLBA <= (sectorLBA + (len - 1) / 512)) {
        flpy_lbaToChs(curSecLBA, &cyl, &head, &sector);

        if ((sectorLBA + (len - 1) / 512 + 1) / 36 == curSecLBA / 36) {
            if (!flpy_transfer_sectors(inst->drive, head, cyl, sector, ((len - 1) / 512 + 1) - (curSecLBA - sectorLBA), floppy_dir_read))
                return false;
            memcpy(addr + (curSecLBA - sectorLBA) * 512, (void*)FLPY_DMA_BUFFER, len - (curSecLBA - sectorLBA) * 512);
            //flpy_controlMotor(inst->drive, false);
            lastAccessedSector = inst->drive;
            lastAccessedSector = curSecLBA;
            lastAccessedSize = ((len - 1) / 512 + 1) - (curSecLBA - sectorLBA);
            useLastAccessed = true;
            return true;
        } else {
            if (!flpy_transfer_sectors(inst->drive, head, cyl, sector, (curSecLBA / 36 + 1) * 36 - curSecLBA, floppy_dir_read))
                return false;
            memcpy(addr + (curSecLBA - sectorLBA) * 512, (void*)FLPY_DMA_BUFFER, ((curSecLBA / 36 + 1) * 36 - curSecLBA) * 512);
            curSecLBA += (curSecLBA / 36 + 1) * 36 - curSecLBA;
        }
    }

    //flpy_controlMotor(inst->drive, false);
    return true;
}

/**
 * @brief Writes contents of the given buffer to the floppy disk.
 * 
 * @param inst a pointer to the instance of a floppy disk
 * @param sectorLBA first sector to write to
 * @param addr buffer to write to the floppy disk
 * @param len number of bytes to write to the floppy disk
 * @return true the write operation was successful.
 * @return false the write operation failed.
 */
bool flpydsk_write_sectors(Flpydsk_t* inst, uint32_t sectorLBA, uint8_t* addr, size_t len) {
    if (inst == NULL) return false;
    if (inst->drive > 3) return false;
    if (len == 0) return true;
    // TODO: fix issues if len is not divisible by 512
    uint8_t head = 0, cyl = 0, sector = 1;
    uint32_t curSecLBA = sectorLBA;

    while (curSecLBA <= (sectorLBA + (len - 1) / 512)) {
        flpy_lbaToChs(curSecLBA, &cyl, &head, &sector);

        if ((sectorLBA + (len - 1) / 512 + 1) / 36 == curSecLBA / 36) {  // if curSecLBA is in the last cylinder to transfer
            //printf("flpy_write_sectors if curSecLBA: %u\thead: %u\tcyl: %u\tsector: %u\t numOfSect: %u\n", curSecLBA, head, cyl, sector, ((len-1) / 512 + 1) - (curSecLBA-sectorLBA));
            memcpy((void*)FLPY_DMA_BUFFER, addr + (curSecLBA - sectorLBA) * 512, len - (curSecLBA - sectorLBA) * 512);
            memset((void*)(FLPY_DMA_BUFFER + len - (curSecLBA - sectorLBA) * 512), 0, 512 - (len - (curSecLBA - sectorLBA) * 512));
            if (!flpy_transfer_sectors(inst->drive, head, cyl, sector, ((len - 1) / 512 + 1) - (curSecLBA - sectorLBA), floppy_dir_write))
                return false;
            //flpy_controlMotor(inst->drive, false);
            lastAccessedSector = inst->drive;
            lastAccessedSector = curSecLBA;
            lastAccessedSize = ((len - 1) / 512 + 1) - (curSecLBA - sectorLBA);
            useLastAccessed = true;
            return true;
        } else {  // if curSecLBA is NOT in the last cylinder
            //printf("flpy_write_sectors else curSecLBA: %u\thead: %u\tcyl: %u\tsector: %u\t numOfSect: %u\n", curSecLBA, head, cyl, sector, (curSecLBA/36 + 1) * 36 - curSecLBA);
            memcpy((void*)FLPY_DMA_BUFFER, addr + (curSecLBA - sectorLBA) * 512, ((curSecLBA / 36 + 1) * 36 - curSecLBA) * 512);
            if (!flpy_transfer_sectors(inst->drive, head, cyl, sector, (curSecLBA / 36 + 1) * 36 - curSecLBA, floppy_dir_write))
                return false;
            curSecLBA += (curSecLBA / 36 + 1) * 36 - curSecLBA;
        }
    }

    //flpy_controlMotor(inst->drive, false);
    return true;
}

void flpy_resetController() {
    flpy_writeDOR(0x00);                                           // disable controller
    flpy_writeDOR(FLPYDSK_DOR_MASK_RESET | FLPYDSK_DOR_MASK_DMA);  // enable controller and enable IRQ and DMA

    flpy_irqWait();

    uint8_t st0, cyl;
    for (uint8_t i = 0; i < 4; ++i) {
        flpy_checkInterrupt(&st0, &cyl);
    }

    outportb(FLPYDSK_CTRL, 0);  // set transfer speed to 500kb/s
}

void flpy_handler(registers_t* r) {
    _irqFired = true;
}

uint8_t flpy_readVersion() {
    flpy_sendCommand(FDC_CMD_VERSION);
    return (flpy_readData());
}

/**
 * @brief Create a new instance of a floppy disk
 * 
 * @param drive floppy drive number (0 - 3)
 * @return Flpydsk_t* pointer to the instance or NULL if something goes wrong
 */
Flpydsk_t* flpydsk_create(uint8_t drive) {
    if (flpy_readVersion() != 0x90)
        return NULL;

    Flpydsk_t* inst = (Flpydsk_t*)malloc(sizeof(Flpydsk_t), 0);
    inst->drive = drive;
    irq_install_handler(6, flpy_handler);
    puts("Resetting controller...");
    flpy_resetController();
    puts("Done\nCalibrating...");
    flpy_calibrate(drive);
    puts("Done\n");
    return inst;
}

/**
 * @brief Destroy the given instance of a floppy disk
 * 
 * @param inst pointer to the instance
 */
void flpydsk_destroy(Flpydsk_t* inst) {
    flpy_controlMotor(inst->drive, false);
    if (inst) free(inst);
}
