#include "paging.h"
#include "kheap.h"
#include "math.h"
#include "os.h"

enum e820_type {
    E820_TYPE_FREE     = 1,
    E820_TYPE_RESERVED = 2,
    E820_TYPE_ACPI     = 3,
    E820_TYPE_NVS      = 4,
    E820_TYPE_BAD      = 5
};

typedef struct
{
    uint64_t base; /**< The address of the region */
    uint64_t size; /**< The size of the region */
    uint32_t type; /**< The e820_type of the region */
    uint32_t ext;
} __attribute__((packed)) mem_map_entry_t;

page_directory_t* active_pd = NULL;
page_directory_t* kernel_pd = NULL;
uint32_t* phys_reservationTable = NULL;
uint32_t phys_reservationTable_size = 0;
uint32_t phys_reservationTable_index = 0;

#define KERNEL_AREA_SIZE 0x1400000  // First 20 MiB
#define MEMORY_MAP_ADDRESS 0x1000
#define MEMORY_MAP_SIZE_ADDRESS 0xFFE

#define FOUR_GB 0x100000000ull

void paging_toggle(bool enable) {
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0"
                     : "=r"(cr0));
    cr0 = (cr0 & ~0x80000000) | (enable << 31);
    __asm__ volatile("mov %0, %%cr0" ::"r"(cr0));
}

uint32_t paging_getPhysAddr(page_directory_t* pd, void* virtAddr) {
    uint32_t pagenr = (uint32_t)virtAddr >> 12;

    if (pd->entries[pagenr >> 10].present) {
        page_table_t* table = pd->virtTables[pagenr >> 10];
        page_table_entry_t tableEntry = table->entries[pagenr & 0x3FF];
        if (tableEntry.present)
            return (tableEntry.page_phys_addr << 12) | ((uint32_t)virtAddr & 0xFFF);
    }
    return NULL;
}

/**
 * @brief Switch to the given page directory
 * 
 * @param pd The page directory to switch to
 */
void paging_switch(page_directory_t* pd) {
    if (pd == NULL)
        pd = kernel_pd;

    if (pd != active_pd) {
        active_pd = pd;
        __asm__ volatile("mov %0, %%cr3" : : "r"(paging_getPhysAddr(active_pd, pd)));
    }
}

uint32_t physAlloc() {
    for (; phys_reservationTable_index < phys_reservationTable_size; ++phys_reservationTable_index) {
        if (phys_reservationTable[phys_reservationTable_index] != 0xFFFFFFFF) {
            uint32_t bitnr;
            if (phys_reservationTable[phys_reservationTable_index] == 0)
                bitnr = 0;
            else
                bitnr = bitScanReverse(phys_reservationTable[phys_reservationTable_index]) + 1;
            phys_reservationTable[phys_reservationTable_index] |= (1 << bitnr);
            return ((phys_reservationTable_index << 5) + bitnr) << 12;
        }
    }

    return NULL;
}

void physSetBits(uint32_t addr_begin, uint32_t addr_end, bool reserved) {
    // Calculate the bit-numbers
    uint32_t start = alignUp(addr_begin, PAGESIZE) / PAGESIZE;
    uint32_t end   = alignDown(addr_end, PAGESIZE) / PAGESIZE;

    if (start == end)
        return;

    if ((!reserved) && (start / 32 < phys_reservationTable_index))
        phys_reservationTable_index = start / 32;

    // Set all these bits
    for (uint32_t j = start; j < end; ++j) {
        if (reserved)
            phys_reservationTable[j / 32] |= 1 << (j % 32);
        else
            phys_reservationTable[j / 32] &= ~(1 << (j % 32));
    }
}

void physSetBit(uint32_t addr, bool reserved) {
    uint32_t pagenr = alignUp(addr, PAGESIZE) / PAGESIZE;

    if ((!reserved) && (pagenr / 32 < phys_reservationTable_index))
        phys_reservationTable_index = pagenr / 32;

    phys_reservationTable[pagenr / 32] = (phys_reservationTable[pagenr / 32] & ~(1 << (pagenr % 32))) | (reserved << (pagenr % 32));
}

bool physGetBit(uint32_t addr) {
    uint32_t pagenr = alignUp(addr, PAGESIZE) / PAGESIZE;

    if (pagenr >= phys_reservationTable_size)
        return false;

    return (phys_reservationTable[pagenr / 32] | (1 << (pagenr % 32))) >> (pagenr % 32);
}

/**
 * @brief Allocate memory of given size at given virtual address, allocate physical memory and map it.
 * 
 * @param pd The targeted page directory
 * @param addr The virtual address to allocate memory at (must be aligned to the PAGESIZE)
 * @param size The number of bytes to allocate (must be aligned to the PAGESIZE)
 * @param flags The flags
 * @return true Successfully allocated memory
 * @return false Failed to allocate memory
 */
bool paging_allocVirt(page_directory_t* pd, void* addr, size_t size, uint32_t flags) {
    ASSERT(((uint32_t)addr % PAGESIZE) == 0);
    ASSERT((size % PAGESIZE) == 0);

    if (pd == NULL)
        pd = kernel_pd;

    uint32_t pageCount = size >> 12;

    for (uint32_t i = 0; i < pageCount; ++i) {
        uint32_t pagenr = ((uint32_t)addr >> 12) + i;
        page_directory_entry_t* pde = &(pd->entries[pagenr >> 10]);  // Pointer because otherwise it would make a local copy and not write to the original address
        page_table_t* table;
        if (pde->present) {
            table = pd->virtTables[pagenr >> 10];
            if (table->entries[pagenr & 0x3FF].present) {
                printf("Page at virtual address: %X already exists", pagenr << 12);
                paging_freeVirt(pd, addr, i * PAGESIZE);
                return false;
            }
        } else {
            table = malloc(sizeof(page_table_t), PAGESIZE);
            memset(table, 0, sizeof(page_table_t));
            pde->page_table_phys_addr = paging_getPhysAddr(kernel_pd, table) >> 12;
            pde->present = true;
            pde->writable = true;
            pde->privilege = (flags & MEM_USER) == MEM_USER;
            pd->virtTables[pagenr >> 10] = table;
        }

        table->values[pagenr & 0x3FF] = physAlloc() | (flags & 0xFFF) | MEM_PRESENT;
    }

    return true;
}

bool paging_allocIdentMap(page_directory_t* pd, void* addr, size_t size, uint32_t flags) {
    ASSERT(((uint32_t)addr % PAGESIZE) == 0);
    ASSERT((size % PAGESIZE) == 0);

    if (pd == NULL)
        pd = kernel_pd;

    uint32_t pageCount = size >> 12;

    for (uint32_t i = 0; i < pageCount; ++i) {
        uint32_t pagenr = ((uint32_t)addr >> 12) + i;

        if (physGetBit(pagenr * PAGESIZE)) {
            printf("Physical page at: %X already allocated", pagenr * PAGESIZE);
            paging_freeVirt(pd, addr, i * PAGESIZE);
            return false;
        }

        page_directory_entry_t* pde = &(pd->entries[pagenr >> 10]);  // Pointer because otherwise it would make a local copy and not write to the original address
        page_table_t* table;
        if (pde->present) {
            table = pd->virtTables[pagenr >> 10];
            if (table->entries[pagenr & 0x3FF].present) {
                printf("Page at virtual address: %X already exists", pagenr << 12);
                paging_freeVirt(pd, addr, i * PAGESIZE);
                return false;
            }
        } else {
            table = malloc(sizeof(page_table_t), PAGESIZE);
            memset(table, 0, sizeof(page_directory_t));
            pde->page_table_phys_addr = paging_getPhysAddr(kernel_pd, table) >> 12;
            pde->present = true;
            pde->writable = true;
            pde->privilege = (flags & MEM_USER) == MEM_USER;
            pd->virtTables[pagenr >> 10] = table;
        }

        physSetBit(pagenr, true);
        table->values[pagenr & 0x3FF] = (uint32_t)(pagenr * PAGESIZE) | (flags & 0xFFF) | MEM_PRESENT;
    }

    return true;
}

/**
 * @brief Free virtual and physical memory at the given virtual address and unmap it.
 * 
 * @param pd The targeted page directory
 * @param addr The virtual address to free (must be aligned to the PAGESIZE)
 * @param size The number of bytes to free (must be aligned to the PAGESIZE)
 */
void paging_freeVirt(page_directory_t* pd, void* addr, size_t size) {
    ASSERT(((uint32_t)addr % PAGESIZE) == 0);
    ASSERT((size % PAGESIZE) == 0);

    if (pd == NULL)
        pd = kernel_pd;

    uint32_t pagenr = (uint32_t)addr >> 12;
    uint32_t pagelast = pagenr + (size >> 12);

    for (; pagenr < pagelast; ++pagenr) {
        page_directory_entry_t* pde = &(pd->entries[pagenr >> 10]);  // Pointer because otherwise it would make a local copy and not write to the original address
        if (pde->present) {
            page_table_t* table = (page_table_t*)(pde->page_table_phys_addr << 12);
            uint32_t phys_addr = table->entries[pagenr & 0x3FF].page_phys_addr << 12;
            table->values[pagenr & 0x3FF] = 0;
            physSetBit(phys_addr, false);
        }
    }
}

void paging_setup_kernel_pd() {
    if (!kernel_pd)
        kernel_pd = malloc(sizeof(page_directory_t), PAGESIZE);

    memset(kernel_pd, 0, sizeof(page_directory_t));

    // Setup the page tables for the first KERNEL_AREA_SIZE bytes (Identity Mapping)
    for (uint32_t i = 0; i < (KERNEL_AREA_SIZE >> 22); ++i) {
        page_table_t* table = malloc(sizeof(page_table_t), PAGESIZE);
        kernel_pd->values[i] = (uint32_t)table | MEM_PRESENT | MEM_WRITABLE;
        kernel_pd->virtTables[i] = table;

        uint32_t pagesLeft = MIN((KERNEL_AREA_SIZE >> 12) - (i << 10), 1024);

        physSetBits(i << 22, (i << 22) + pagesLeft * PAGESIZE, true);

        uint16_t j;
        for (j = 0; j < pagesLeft; ++j) {
            table->values[j] = (i << 22) | (j << 12);
            table->entries[j].present = true;
            table->entries[j].writable = true;
        }

        for (; j < 1024; ++j)
            table->values[j] = 0;
    }

    // Setup the page directory for the kernel heap unmapped (KERNEL_HEAP_START - 4 GiB)
    for (uint16_t i = KERNEL_HEAP_START >> 22; i < 1024; ++i) {
        page_table_t* table = malloc(sizeof(page_table_t), PAGESIZE);
        kernel_pd->values[i] = (uint32_t)table | MEM_PRESENT | MEM_WRITABLE;
        kernel_pd->virtTables[i] = table;
        memset(table, 0, sizeof(page_table_t));
    }
}

uint32_t physMemInit() {
    mem_map_entry_t* memoryMap = (mem_map_entry_t*)MEMORY_MAP_ADDRESS;
    uint16_t memoryMapSize = *((uint16_t*)MEMORY_MAP_SIZE_ADDRESS);

    for (uint32_t i = 0; i < memoryMapSize; ++i) {
        mem_map_entry_t entry = memoryMap[i];

        if (entry.base < FOUR_GB && (entry.base + entry.size) > FOUR_GB) {
            entry.size = FOUR_GB - entry.base;
        }

        if (entry.type == E820_TYPE_FREE)
            phys_reservationTable_size = MAX(phys_reservationTable_size, (entry.base + entry.size) >> 17);
    }

    phys_reservationTable = malloc(phys_reservationTable_size * 4, 4);
    memset(phys_reservationTable, 0xFF, phys_reservationTable_size * 4);

    for (uint32_t i = 0; i < memoryMapSize; ++i) {
        mem_map_entry_t entry = memoryMap[i];
        if (entry.base < FOUR_GB) {
            printf("base: %X%X\tsize: %X%X\ttype: %u\n", entry.base, entry.size, entry.type);
            if (entry.type == E820_TYPE_FREE)
                physSetBits((uint32_t)entry.base, (uint32_t)(entry.base + entry.size), false);
        }
    }

    return phys_reservationTable_size << 17;
}

/**
 * @brief Retrieve the currently active page directory.
 * 
 * @return page_directory_t* the currently active page directory
 */
page_directory_t* paging_getActivePageDirectory() {
    return active_pd;
}

/**
 * @brief Create a new page directory which is a copy of the kernel page directory.
 * 
 * @return page_directory_t* The newly created page directory
 */
page_directory_t* paging_createPageDirectory() {
    page_directory_t* pd = malloc(sizeof(page_directory_t), PAGESIZE);
    memcpy(pd, kernel_pd, sizeof(page_directory_t));
    pd->values[1] = 0;  // TODO: Change this (this is temporary because of 0x400000)
    return pd;
}

/**
 * @brief Destroy the given page directory and free all physical memory associated with it, except for kernel pages.
 * 
 * @param pd The page directory to destroy
 */
void paging_destroyPageDirectory(page_directory_t* pd) {
    ASSERT(pd != kernel_pd);

    if (pd == active_pd)
        paging_switch(kernel_pd);

    for (uint32_t i = 0; i < 1024; ++i) {
        // Don't free page tables and memory of the kernel area
        if ((pd->entries[i].present) && (pd->virtTables[i] != kernel_pd->virtTables[i])) {
            page_table_t* table = pd->virtTables[i];
            for (uint32_t j = 0; j < 1024; ++j) {
                if (table->entries[j].present)
                    physSetBit(table->values[j] & 0xFFFFF000, false);
            }

            free(table);
        }
    }

    free(pd);
}

uint32_t paging_install() {
    uint32_t mem_available = physMemInit();
    printf("mem_available: %u", mem_available);

    paging_setup_kernel_pd();

    paging_switch(kernel_pd);
    paging_toggle(true);

    return mem_available;
}
