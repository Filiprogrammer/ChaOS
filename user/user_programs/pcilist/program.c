#include "pciVendProdList.h"
#include "userlib.h"

int main() {
    settextcolor(0xF, 0x0);
    puts("PCI List:\nVendor\t\t    Device\t   Description\t\t\t\t     IRQ\n");
    settextcolor(0xB, 0x0);
    for (uint32_t i = 1; i <= 8192; ++i) {
        pciDev_t pciDev = {0};
        if (pci_getDevice(i, &pciDev)) {
            for (uint32_t j = 0; j < PCI_VENTABLE_LEN; ++j) {
                if (PciVenTable[j].VenId == pciDev.vendorID) {
                    puts((char*)PciVenTable[j].VenFull);
                    break;
                }
            }

            for (uint32_t j = 0; j < PCI_DEVTABLE_LEN; ++j) {
                if (PciDevTable[j].VenId == pciDev.vendorID && PciDevTable[j].DevId == pciDev.deviceID) {
                    putch('\r');
                    for(uint8_t k = 0; k < 20; ++k)
                        move_cursor_right();
                    
                    puts((char*)PciDevTable[j].Chip);

                    putch('\r');
                    for(uint8_t k = 0; k < 35; ++k)
                        move_cursor_right();

                    puts((char*)PciDevTable[j].ChipDesc);
                    break;
                }
            }

            putch('\r');
            for(uint8_t k = 0; k < 77; ++k)
                move_cursor_right();

            char irq_str[] = {0, 0, 0, 0};
            uitoa(pciDev.irq, irq_str);
            puts(irq_str);
            putch('\n');
        } else {
            break;
        }
    }
    return 0;
}
