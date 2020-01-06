#include "pciVendProdList.h"
#include "userlib.h"

int main() {
    puts("PCI List:\n");
    for (uint32_t i = 1; i <= 8192; ++i) {
        pciDev_t pciDev = {0};
        if (pci_getDevice(i, &pciDev)) {
            puts("Vendor: ");
            for (uint32_t j = 0; j < PCI_VENTABLE_LEN; ++j) {
                if (PciVenTable[j].VenId == pciDev.vendorID) {
                    puts((char*)PciVenTable[j].VenFull);
                    break;
                }
            }
            puts("\tIRQ: ");
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
