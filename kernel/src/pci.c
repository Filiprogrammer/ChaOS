#include "pci.h"
#include "os.h"
#include "ehci.h"

listHead_t* pciDevList = NULL;

void pci_scanBus(uint8_t bus);

uint8_t pci_configReadByte(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    pci_config_addr_reg_t addr;
    addr.busNumb = bus;
    addr.devNumb = slot;
    addr.funcNumb = func;
    addr.registerOffset = offset & 0xFC;
    addr.enable = true;

    outportl(PCI_CONFIG_ADDRESS, (uint32_t)(*(uint32_t*)&addr));

    uint32_t tmp = inportl(PCI_CONFIG_DATA);

    return (tmp >> ((offset % 4) * 8));
}

uint16_t pci_configReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    pci_config_addr_reg_t addr;
    addr.busNumb = bus;
    addr.devNumb = slot;
    addr.funcNumb = func;
    addr.registerOffset = offset & 0xFC;
    addr.enable = true;

    outportl(PCI_CONFIG_ADDRESS, (uint32_t)(*(uint32_t*)&addr));

    uint32_t tmp = inportl(PCI_CONFIG_DATA);

    return (tmp >> ((offset % 4) * 8));
}

uint32_t pci_configReadDword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    pci_config_addr_reg_t addr;
    addr.busNumb = bus;
    addr.devNumb = slot;
    addr.funcNumb = func;
    addr.registerOffset = offset & 0xFC;
    addr.enable = true;

    outportl(PCI_CONFIG_ADDRESS, (uint32_t)(*(uint32_t*)&addr));

    return inportl(PCI_CONFIG_DATA);
}

void pci_configWriteByte(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint8_t val) {
    pci_config_addr_reg_t addr;
    addr.busNumb = bus;
    addr.devNumb = slot;
    addr.funcNumb = func;
    addr.registerOffset = offset & 0xFC;
    addr.enable = true;

    outportl(PCI_CONFIG_ADDRESS, (uint32_t)(*(uint32_t*)&addr));

    outportb(PCI_CONFIG_DATA + (offset & 0x03), val);
}

void pci_configWriteWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t val) {
    pci_config_addr_reg_t addr;
    addr.busNumb = bus;
    addr.devNumb = slot;
    addr.funcNumb = func;
    addr.registerOffset = offset & 0xFC;
    addr.enable = true;

    outportl(PCI_CONFIG_ADDRESS, (uint32_t)(*(uint32_t*)&addr));

    outportw(PCI_CONFIG_DATA + (offset & 0x01), val);
}

void pci_configWriteDword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t val) {
    pci_config_addr_reg_t addr;
    addr.busNumb = bus;
    addr.devNumb = slot;
    addr.funcNumb = func;
    addr.registerOffset = offset & 0xFC;
    addr.enable = true;

    outportl(PCI_CONFIG_ADDRESS, (uint32_t)(*(uint32_t*)&addr));

    outportw(PCI_CONFIG_DATA + offset, val);
}

void pci_checkFunction(uint8_t bus, uint8_t dev, uint8_t func) {
    uint8_t class = pci_configReadByte(bus, dev, func, PCI_CLASS);
    uint8_t subClass = pci_configReadByte(bus, dev, func, PCI_SUBCLASS);
    uint8_t secondaryBus;

    uint16_t vendorID = pci_configReadWord(bus, dev, func, PCI_VENDOR_ID);

    if(vendorID != 0xFFFF) {
        pciDev_t* pciDev = malloc(sizeof(pciDev_t), 0);
        pciDev->bus = bus;
        pciDev->device = dev;
        pciDev->func = func;
        pciDev->vendorID = vendorID;
        pciDev->deviceID = pci_configReadWord(bus, dev, func, PCI_DEVICE_ID);
        pciDev->class = pci_configReadByte(bus, dev, func, PCI_CLASS);
        pciDev->subclass = pci_configReadByte(bus, dev, func, PCI_SUBCLASS);
        pciDev->interfaceID = pci_configReadByte(bus, dev, func, PCI_PROG_IF);
        pciDev->interruptPin = pci_configReadByte(bus, dev, func, PCI_INTERRUPT_PIN);
        pciDev->irq = pci_configReadByte(bus, dev, func, PCI_INTERRUPT_LINE);
        pciDev->revID = pci_configReadByte(bus, dev, func, PCI_REVISION_ID);

        uint8_t headerType = pci_configReadByte(bus, dev, 0, PCI_HEADER_TYPE);

        // Read BARs
        if((headerType & 0x7F) == 0 || (headerType & 0x7F) == 1) {
            uint8_t barCount = 0;

            if((headerType & 0x7F) == 0)
                barCount = 6;
            else if((headerType & 0x7F) == 1)
                barCount = 2;

            for(uint8_t i = 0; i < 6; ++i) {
                if (i >= barCount) {
                    pciDev->bar[i].memoryType = PCI_INVALID_BAR;
                    continue;
                }

                pciDev->bar[i].baseAddress = pci_configReadDword(bus, dev, func, 0x10 + i * 4);

                // Check if BAR is valid
                if(pciDev->bar[i].baseAddress) {
                    uint32_t mask;

                    if(pciDev->bar[i].baseAddress & 0x1) {
                        pciDev->bar[i].memoryType = PCI_IO_SPACE; // I/O Space
                        mask = 0xFFFFFFFC;
                    } else {
                        pciDev->bar[i].memoryType = PCI_MEM_SPACE; // Memory Space
                        mask = 0xFFFFFFF0;
                    }

                    // Check memory size
                    cli();
                    pci_configWriteDword(bus, dev, func, 0x10 + i * 4, 0xFFFFFFFF);
                    uint32_t tmp = pci_configReadDword(bus, dev, func, 0x10 + i * 4);
                    pciDev->bar[i].memorySize = (~(tmp & mask)) + 1;
                    sti();

                    pciDev->bar[i].baseAddress &= mask;
                } else {
                    pciDev->bar[i].memoryType = PCI_INVALID_BAR;
                }
            }
        }

        list_append(pciDevList, pciDev);

        if((class == 0x06) && (subClass == 0x04)) {
            uint8_t headerType = pci_configReadByte(bus, dev, 0, PCI_HEADER_TYPE);

            if((headerType & 0x7F) == 0) {
                secondaryBus = pci_configReadByte(bus, dev, func, 0x19);
                pci_scanBus(secondaryBus);
            }
        } else if((class == 0x0C) && (subClass == 0x03)) {
            // USB Controller
            if(pciDev->interfaceID == 0x00) {
                // UHCI
            } else if(pciDev->interfaceID == 0x10) {
                // OHCI
            } else if(pciDev->interfaceID == 0x20) {
                // EHCI (USB2)
                ehci_create(pciDev);
            } else if(pciDev->interfaceID == 0x30) {
                // XHCI (USB3)
            }
        }
    }
}

void pci_checkDevice(uint8_t bus, uint8_t dev) {
    uint8_t headerType = pci_configReadByte(bus, dev, 0, PCI_HEADER_TYPE);
    uint8_t funcCount = 8;

    // if device is not multifunctional
    if (!(headerType & 0x80))
        funcCount = 1; // not multifunctional, only function 0 used

    for(uint8_t i = 0; i < funcCount; ++i) {
        pci_checkFunction(bus, dev, i);
    }
}

void pci_scanBus(uint8_t bus) {
    for(uint8_t i = 0; i < 32; ++i) {
        pci_checkDevice(bus, i);
    }
}

/**
 * @brief Scan the PCI device tree
 * 
 */
void pci_scan() {
    if(pciDevList == NULL)
        pciDevList = list_create();

    uint8_t headerType = pci_configReadByte(0, 0, 0, PCI_HEADER_TYPE);

    // if device is multifunctional
    if (headerType & 0x80) {
        // Multiple PCI Host controllers
        for(uint8_t func = 0; func < 8; ++func) {
            if(pci_configReadWord(0, 0, func, PCI_VENDOR_ID) != 0xFFFF)
                break;

            uint8_t bus = func;
            pci_scanBus(bus);
        }
    } else {
        // Single PCI Host controller
        pci_scanBus(0);
    }
}

/**
 * @brief Get information about a PCI device.
 * 
 * @param i index of the PCI device
 * @param pciDev pointer to a block of memory where to store the resulting pciDev_t structure
 * @return true if device at the given index was found.
 * @return false if no device at the given index was found.
 */
bool pci_getDevice(uint32_t i, pciDev_t* pciDev) {
    if(pciDevList && pciDev) {
        pciDev_t* pciDevice = list_getElement(pciDevList, i);

        if(pciDevice == NULL)
            return false;

        memcpy(pciDev, pciDevice, sizeof(pciDev_t));
        return true;
    }

    return false;
}
