#include "pci.h"

listHead_t* pciDev_List;

uint32_t pci_config_read( uint8_t bus, uint8_t device, uint8_t func, uint16_t content )
{
    // example: PCI_VENDOR_ID 0x0200 ==> length: 0x02 reg: 0x00 offset: 0x00
    uint8_t length  = content >> 8;
    uint8_t reg_off = content & 0x00FF;
    uint8_t reg     = reg_off & 0xFC;     // bit mask: 11111100b
    uint8_t offset  = reg_off % 0x04;     // remainder of modulo operation provides offset

    outportl(PCI_CONFIGURATION_ADDRESS,
        0x80000000
        | (bus    << 16)
        | (device << 11)
        | (func   <<  8)
        | (reg         ));

    // use offset to find searched content
    uint32_t readVal = inportl(PCI_CONFIGURATION_DATA) >> (8 * offset);

    switch(length)
    {
        case 1:
            readVal &= 0x000000FF;
        break;
        case 2:
            readVal &= 0x0000FFFF;
        break;
        case 4:
            readVal &= 0xFFFFFFFF;
        break;
    }
    return readVal;
}

void pci_config_write_dword( uint8_t bus, uint8_t device, uint8_t func, uint8_t reg, uint32_t val )
{
    outportl(PCI_CONFIGURATION_ADDRESS,
        0x80000000
        | (bus     << 16)
        | (device  << 11)
        | (func    <<  8)
        | (reg & 0xFC   ));

    outportl(PCI_CONFIGURATION_DATA, val);
}

void pciScan()
{
    if(!pciDev_List) pciDev_List = list_create();
    
    uint32_t i;
    uint8_t  bus                = 0; // max. 256
    uint8_t  device             = 0; // max.  32
    uint8_t  func               = 0; // max.   8

    uint32_t pciBar             = 0; // helper variable for memory size
    uint32_t EHCI_data          = 0; // helper variable for EHCI_data

    for(bus=0;bus<8;++bus)
    {
        for(device=0;device<32;++device)
        {
            for(func=0;func<8;++func)
            {
                uint16_t vendorID = pci_config_read( bus, device, func, PCI_VENDOR_ID);
                if( vendorID && (vendorID != 0xFFFF) )
                {
                    pciDev_t* pciDev = malloc(sizeof(pciDev_t), 0);
                    pciDev->vendorID     = vendorID;
                    pciDev->deviceID     = pci_config_read( bus, device, func, PCI_DEVICE_ID  );
                    pciDev->classID      = pci_config_read( bus, device, func, PCI_CLASS      );
                    pciDev->subclassID   = pci_config_read( bus, device, func, PCI_SUBCLASS   );
                    pciDev->interfaceID  = pci_config_read( bus, device, func, PCI_INTERFACE  );
                    pciDev->revID        = pci_config_read( bus, device, func, PCI_REVISION   );
                    pciDev->irq          = pci_config_read( bus, device, func, PCI_IRQLINE    );
                    pciDev->bar[0].baseAddress = pci_config_read( bus, device, func, PCI_BAR0 );
                    pciDev->bar[1].baseAddress = pci_config_read( bus, device, func, PCI_BAR1 );
                    pciDev->bar[2].baseAddress = pci_config_read( bus, device, func, PCI_BAR2 );
                    pciDev->bar[3].baseAddress = pci_config_read( bus, device, func, PCI_BAR3 );
                    pciDev->bar[4].baseAddress = pci_config_read( bus, device, func, PCI_BAR4 );
                    pciDev->bar[5].baseAddress = pci_config_read( bus, device, func, PCI_BAR5 );

                    // Valid Device
                    pciDev->bus    = bus;
                    pciDev->device = device;
                    pciDev->func   = func;

                    // output to screen
                    printf("%d:%d.%d\t dev:%x vend:%x",
                        pciDev->bus, pciDev->device, pciDev->func, pciDev->deviceID, pciDev->vendorID );

                    if(pciDev->irq!=255)
                    {
                        printf(" IRQ:%d ", pciDev->irq );
                    }
                    else // "255 means "unknown" or "no connection" to the interrupt controller"
                    {
                        puts(" IRQ:-- ");
                    }

                    // test on USB
                    if( (pciDev->classID == 0x0C) && (pciDev->subclassID == 0x03) )
                    {
                        puts(" USB ");
                        if( pciDev->interfaceID==0x00 ) { puts("UHCI ");   }
                        else if( pciDev->interfaceID==0x10 ) { puts("OHCI ");   }
                        else if( pciDev->interfaceID==0x20 ) { puts("EHCI ");   }
                        else if( pciDev->interfaceID==0x30 ) { puts("XHCI ");   }
                        else if( pciDev->interfaceID==0x80 ) { puts("no HCI "); }
                        else if( pciDev->interfaceID==0xFE ) { puts("any HCI ");    }

                        for(i = 0; i < 6; ++i) // check USB BARs
                        {
                            pciDev->bar[i].memoryType = pciDev->bar[i].baseAddress & 0x01;

                            if(pciDev->bar[i].baseAddress) // check valid BAR
                            {
                                if(pciDev->bar[i].memoryType == 0)
                                {
                                    printf("%d:%X MEM ", i, pciDev->bar[i].baseAddress & 0xFFFFFFF0 );
                                }
                                if(pciDev->bar[i].memoryType == 1)
                                {
                                    printf("%d:%X I/O ", i, pciDev->bar[i].baseAddress & 0xFFFFFFFC );
                                }

                                /// TEST Memory Size Begin
                                cli();
                                pci_config_write_dword  ( bus, device, func, PCI_BAR0 + 4*i, 0xFFFFFFFF );
                                pciBar = pci_config_read( bus, device, func, PCI_BAR0 + 4*i             );
                                pci_config_write_dword  ( bus, device, func, PCI_BAR0 + 4*i,
                                                          pciDev->bar[i].baseAddress       );
                                sti();
                                pciDev->bar[i].memorySize = (~pciBar | 0x0F) + 1;
                                printf("sz:%d ", pciDev->bar[i].memorySize );
                                /// TEST Memory Size End

                                /// TEST EHCI Data Begin
                                if(  (pciDev->interfaceID==0x20)   // EHCI
                                   && pciDev->bar[i].baseAddress ) // valid BAR
                                {
                                    /*
                                    Offset Size Mnemonic    Power Well   Register Name
                                    00h     1   CAPLENGTH      Core      Capability Register Length
                                    01h     1   Reserved       Core      N/A
                                    02h     2   HCIVERSION     Core      Interface Version Number
                                    04h     4   HCSPARAMS      Core      Structural Parameters
                                    08h     4   HCCPARAMS      Core      Capability Parameters
                                    0Ch     8   HCSP-PORTROUTE Core      Companion Port Route Description
                                    */

                                    uint32_t bar = pciDev->bar[i].baseAddress & 0xFFFFFFF0;

                                    EHCI_data = *((volatile uint8_t* )(bar + 0x00));
                                    printf("\nBAR%d CAPLENGTH:  %x \t\t",i, EHCI_data);

                                    EHCI_data = *((volatile uint16_t*)(bar + 0x02));
                                    printf(  "BAR%d HCIVERSION: %x \n",i, EHCI_data);

                                    EHCI_data = *((volatile uint32_t*)(bar + 0x04));
                                    printf(  "BAR%d HCSPARAMS:  %X \t",i, EHCI_data);

                                    EHCI_data = *((volatile uint32_t*)(bar + 0x08));
                                    printf(  "BAR%d HCCPARAMS:  %X \n",i, EHCI_data);
                                }
                                /// TEST EHCI Data End
                            } // if
                        } // for
                    } // if
                    putch('\n');
                    list_append(pciDev_List, pciDev);
                } // if pciVendor

                // Bit 7 in header type (Bit 23-16) --> multifunctional
                if( !(pci_config_read(bus, device, 0, PCI_HEADERTYPE) & 0x80) )
                {
                    break; // --> not multifunctional, only function 0 used
                }
            } // for function
        } // for device
    } // for bus
    putch('\n');
}

bool pci_getDevice(uint32_t i, pciDev_t* pciDev) {
    if(pciDev_List && pciDev) {
        pciDev_t* pciDevice = list_getElement(pciDev_List, i);
        if(pciDevice == NULL) return false;
        memcpy(pciDev, pciDevice, sizeof(pciDev_t));
        return true;
    }
    return false;
}
