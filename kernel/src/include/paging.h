#ifndef PAGING_H
#define PAGING_H

#include "stdint.h"
#include "stdbool.h"

#define MEM_PRESENT 1
#define MEM_WRITABLE 2
#define MEM_USER 4

typedef struct
{
    uint8_t present : 1;
    uint8_t writable : 1;
    uint8_t privilege : 1;
    uint8_t write_through : 1;
    uint8_t cache_disabled : 1;
    uint8_t accessed : 1;
    uint8_t reserved : 1;
    uint8_t page_size : 1;
    uint8_t ignored : 1;
    uint8_t available : 3;
    uint32_t page_table_phys_addr : 20;
} __attribute__((packed)) page_directory_entry_t;

typedef struct
{
    uint8_t present : 1;
    uint8_t writable : 1;
    uint8_t privilege : 1;
    uint8_t write_through : 1;
    uint8_t cache_disabled : 1;
    uint8_t accessed : 1;
    uint8_t dirty : 1;
    uint8_t reserved : 1;
    uint8_t global : 1;
    uint8_t available : 3;
    uint32_t page_phys_addr : 20;
} __attribute__((packed)) page_table_entry_t;

typedef union
{
    page_table_entry_t entries[1024];
    uint32_t values[1024];
} __attribute__((packed)) page_table_t;

typedef struct {
    // Temporarly commented out because I am still using the c99 standard
    //union
    //{
        //page_directory_entry_t entries[1024];
        uint32_t values[1024];
    //};
    page_table_t* virtTables[1024]; // Virtual for the kernel_pd
} __attribute__((packed)) page_directory_t;

uint32_t paging_install();
void paging_switch(page_directory_t* pd);
bool paging_allocVirt(page_directory_t* pd, void* addr, size_t size, uint32_t flags);
void paging_freeVirt(page_directory_t* pd, void* addr, size_t size);
page_directory_t* paging_getActivePageDirectory();
page_directory_t* paging_createPageDirectory();
void paging_destroyPageDirectory(page_directory_t* pd);

#endif
