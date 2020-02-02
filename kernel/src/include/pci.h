#ifndef PCI_H
#define PCI_H

#include "stdint.h"
#include "stdbool.h"
#include "list.h"

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

#define PCI_VENDOR_ID       0x0
#define PCI_DEVICE_ID       0x2
#define PCI_COMMAND         0x4
#define PCI_STATUS          0x6
#define PCI_REVISION_ID     0x8
#define PCI_PROG_IF         0x9
#define PCI_SUBCLASS        0xA
#define PCI_CLASS           0xB
#define PCI_CACHE_LINE_SIZE 0xC
#define PCI_LATENCY_TIMER   0xD
#define PCI_HEADER_TYPE     0xE
#define PCI_BIST            0xF
#define PCI_INTERRUPT_LINE  0x3C
#define PCI_INTERRUPT_PIN   0x3D

typedef struct {
    uint16_t vendorID;
    uint16_t deviceID;
    uint16_t command;
    uint16_t status;
    uint8_t revisionID;
    uint8_t progIF;
    uint8_t subclass;
    uint8_t class;
    uint8_t cacheLineSize;
    uint8_t latencyTimer;
    uint8_t headerType;
    uint8_t BIST;
    uint32_t BAR0;
    uint32_t BAR1;
    uint32_t BAR2;
    uint32_t BAR3;
    uint32_t BAR4;
    uint32_t BAR5;
    uint32_t cardbusCISPointer;
    uint16_t subsystemVendorID;
    uint16_t subsystemID;
    uint32_t expansionROMBaseAddress;
    uint8_t capabilitiesPointer;
    uint8_t reserved[7];
    uint8_t interruptLine;
    uint8_t interruptPin;
    uint8_t minGrant;
    uint8_t maxLatency;
} __attribute__((packed)) pci_dev_struct_0_t;

typedef struct {
    uint16_t vendorID;
    uint16_t deviceID;
    uint16_t command;
    uint16_t status;
    uint8_t revisionID;
    uint8_t progIF;
    uint8_t subclass;
    uint8_t class;
    uint8_t cacheLineSize;
    uint8_t latencyTimer;
    uint8_t headerType;
    uint8_t BIST;
    uint32_t BAR0;
    uint32_t BAR1;
    uint8_t primaryBusNumber;
    uint8_t secondaryBusNumber;
    uint8_t subordinateBusNumber;
    uint8_t secondaryLatencyTimer;
    uint8_t IOBaseLow;
    uint8_t IOLimitLow;
    uint16_t secondaryStatus;
    uint16_t memBase;
    uint16_t memLimit;
    uint16_t prefetchMemBaseLow;
    uint16_t prefetchMemLimitLow;
    uint32_t prefetchMemBaseHigh;
    uint32_t prefetchMemLimitHigh;
    uint16_t IOBaseHigh;
    uint16_t IOLimitHigh;
    uint8_t capabilityPointer;
    uint8_t reserved[3];
    uint32_t expansionROMBaseAddress;
    uint8_t interruptLine;
    uint8_t interruptPin;
    uint16_t bridgeControl;
} __attribute__((packed)) pci_dev_struct_1_t;

typedef struct {
    uint16_t vendorID;
    uint16_t deviceID;
    uint16_t command;
    uint16_t status;
    uint8_t revisionID;
    uint8_t progIF;
    uint8_t subclass;
    uint8_t class;
    uint8_t cacheLineSize;
    uint8_t latencyTimer;
    uint8_t headerType;
    uint8_t BIST;
    uint32_t cardBusSocketExCaBaseAddress;
    uint8_t capabilitiesListOffset;
    uint8_t reserved;
    uint16_t secondaryStatus;
    uint8_t pciBusNumber;
    uint8_t cardBusBusNumber;
    uint8_t subordinateBusNumber;
    uint8_t cardBusLatencyTimer;
    uint32_t memBaseAddress0;
    uint32_t memLimit0;
    uint32_t memBaseAddress1;
    uint32_t memLimit1;
    uint32_t IOBaseAddress0;
    uint32_t IOLimit0;
    uint32_t IOBaseAddress1;
    uint32_t IOLimit1;
    uint8_t interruptLine;
    uint8_t interruptPin;
    uint16_t bridgeControl;
    uint16_t subsystemDevID;
    uint16_t subsystemVendID;
    uint32_t legacyBaseAddress;
} __attribute__((packed)) pci_dev_struct_2_t;

typedef struct {
    uint8_t registerOffset;
    uint8_t funcNumb : 3;
    uint8_t devNumb : 5;
    uint8_t busNumb;
    uint8_t reserved : 7;
    bool enable : 1;
} pci_config_addr_reg_t;

typedef struct {
    uint32_t baseAddress;
    size_t   memorySize;
    uint8_t  memoryType;
} pciBar_t;

typedef struct {
    uint16_t  vendorID;
    uint16_t  deviceID;
    uint8_t   class;
    uint8_t   subclass;
    uint8_t   interfaceID;
    uint8_t   revID;
    uint8_t   bus;
    uint8_t   device;
    uint8_t   func;
    uint8_t   irq;
    uint8_t   interruptPin;
    pciBar_t  bar[6];
} pciDev_t;

enum {
    PCI_MEM_SPACE,
    PCI_IO_SPACE,
    PCI_INVALID_BAR
};

extern listHead_t* pciDevList;

uint8_t pci_configReadByte(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint16_t pci_configReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint32_t pci_configReadDword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void pci_configWriteByte(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint8_t val);
void pci_configWriteWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t val);
void pci_configWriteDword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t val);
void pci_scan();
bool pci_getDevice(uint32_t i, pciDev_t* pciDev);

#endif
