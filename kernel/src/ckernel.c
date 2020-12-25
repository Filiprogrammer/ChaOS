#include "cmos.h"
#include "fileManager.h"
#include "keyboard.h"
#include "kheap.h"
#include "list.h"
#include "os.h"
#include "paging.h"
#include "partitionManager.h"
#include "pc_speaker.h"
#include "pci.h"
#include "storage_devManager.h"
#include "syscall.h"
#include "task.h"
#include "time.h"

#ifdef __TEST
#include "listTest.h"
#include "queueTest.h"
#endif

// Operating system common Data Area
oda_t ODA;

bool fpu_install() {
    if (!(cmos_read(0x14) & 0x02))
        return false;

    __asm__ volatile("finit");

    // Set the FPU Control Word. FLDCW = Load FPU Control Word
    uint16_t ctrlword = 0x37F;
    __asm__ volatile("fldcw %0"
                     :
                     : "m"(ctrlword));

    // Set the TS bit (no. 3) in CR0 to enable #NM (exception no. 7)
    // Set NS bit (no. 5) to signal FPU exceptions internally instead using PIC
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0"
                     : "=r"(cr0));  // Read cr0
    cr0 |= 0x08 | 0x20;
    __asm__ volatile("mov %0, %%cr0"
                     :
                     : "r"(cr0));  // Write cr0

    return true;
}

static void init() {
    clear_screen();
    settextcolor(14, 0);
    char* welcomemsg = __VERSION_STRING;
    if (BSDChecksum(welcomemsg) == __VERSION_CHECKSUM)
        puts(welcomemsg);
    else
        reboot();
    time_t ptm;
    time_read_rtc(&ptm);
    printf("Date: %d.%d.%d\n", ptm.day, ptm.month, ptm.year);
    printf("Time: %d:%d:%d\n", ptm.hour, ptm.minute, ptm.second);
    gdt_install();
    idt_install();
    timer_install();
    keyboard_install();
    mouse_install();
    syscall_install();
    // paging, kernel heap, tasking
    ODA.Memory_Size = paging_install();
    printf("Main memory: %u MB\n", ODA.Memory_Size / (1024 * 1024));
    heap_install();
    ODA.cpu_frequency = 1000000000;
    tasking_install();
    randomSetSeed(cmos_read(0x00));  //Set the seconds as the seed
    puts("FPU");
    if (fpu_install())
        puts(" installed\n");
    else
        puts(" not found\n");
}

int main() {
    init();

    settextcolor(15, 0);

    pci_scan();  // scan of pci bus; results go to: pciDev_t pciDevList;

    uint32_t pciDevListSize = list_getSize(pciDevList);

    puts("PCI List:\n");

    for (uint32_t i = 0; i < pciDevListSize; ++i) {
        pciDev_t* pciDev = list_getElement(pciDevList, i + 1);
        printf("%d:%d.%d\t dev:%x vend:%x class:%u subclass %u", pciDev->bus, pciDev->device, pciDev->func, pciDev->deviceID, pciDev->vendorID, pciDev->class, pciDev->subclass);

        if (pciDev->irq != 0xFF)
            printf(" IRQ:%u ", pciDev->irq);
        else
            puts(" IRQ:-- ");

        putch('\n');
    }

    sti();

    putch('\n');

    // Storage device detection
    storage_devManager_init();
    listHead_t* storageDevList = storage_getDevList();
    size_t storageDevCount = list_getSize(storageDevList);
    printf("Detected %d storage devices:\n", storageDevCount);
    uint32_t i;
    for (i = 0; i < storageDevCount; ++i) {
        storage_dev_t* curDev = list_getElement(storageDevList, i + 1);
        switch (curDev->type) {
            case 0:
                puts("\t- Floppy");
                break;
            case 1:
                puts("\t- Harddrive");
                break;
        }

        listHead_t* partList = curDev->partitions;
        if (partList == 0) {
            puts("No partitions/partition table detected");
            continue;
        }
        uint32_t partNumber = list_getSize(partList);
        printf(" %d partitions\n", partNumber);
        for (uint8_t j = 0; j < partNumber; ++j) {
            Partition_t* partEntry = list_getElement(partList, j + 1);
            printf("\t\tPartition %d\n", j + 1);
            printf("\t\t- Start sector: %X\n", PartManage_getPartStartSector(partEntry->partEntry));
            printf("\t\t- Length: %X\n", PartManage_getPartLength(partEntry->partEntry));
            puts("\t\t- Type: ");
            switch (PartManage_getPartType(partEntry->partEntry)) {
                case 0x0B:  //FAT32
                    puts("FAT32");
                    break;
                case 0xEE:  //GPT
                    puts("GPT");
                    break;
                case 0x01:  //FAT12
                    puts("FAT12");
                    break;
                default:
                    puts("(undefined)");
                    break;
            }
            putch('\n');
        }
        putch('\n');
    }

    printf("Boot partition: %c%c\n", ODA.bootDev + 'a', ODA.bootPart + '1');

#ifdef __TEST
    puts("Running tests:\n");
    test_list_main();
    test_queue_main();
#endif

    ODA.ts_flag = 1;  // enable task_switching
    puts("Executing STARTUP.ELF...");
    if (file_execute("~~/STARTUP.ELF")) {
        puts("Success\n");

        for (;;)
            sleepCurrentThread(99999);
    } else {
        puts("Failed\n");
    }

    puts("Executing SHELL.ELF...");
    if (file_execute("~~/SHELL.ELF"))
        puts("Success\n");
    else
        puts("Failed\n");

    for (;;)
        sleepCurrentThread(99999);

    return 0;
}
