#include "ehci.h"
#include "paging.h"

void ehci_initHC(ehci_t* ehci);
void ehci_resetHC(ehci_t* ehci);
void ehci_startHC(ehci_t* ehci);
void ehci_disableLegacySupport(ehci_t* ehci);
void ehci_handler(registers_t* r, void* arg);
void ehci_checkPorts(ehci_t* ehci);

void ehci_create(pciDev_t* pciDev) {
    ehci_t* ehci = malloc(sizeof(ehci_t), 0);
    ehci->pciDev = pciDev;

    if(pciDev->bar[0].memoryType == PCI_MEM_SPACE) {
        if(paging_allocIdentMap(NULL, (void*)(pciDev->bar[0].baseAddress), 0x1000, MEM_WRITABLE)) {
            ehci_capRegs_t* capRegs = (ehci_capRegs_t*)pciDev->bar[0].baseAddress;
            printf("EHCI version: %x\n", capRegs->HCIVERSION);

            ehci_opRegs_t* opRegs = (ehci_opRegs_t*)((uint32_t)capRegs + capRegs->CAPLENGTH);

            ehci->opRegs = opRegs;
            ehci->capRegs = capRegs;

            puts("ehci_initHC...\n");
            ehci_initHC(ehci);
            puts("ehci_initHC done\n");

            puts("ehci_resetHC...\n");
            ehci_resetHC(ehci);
            puts("ehci_resetHC done\n");

            puts("ehci_startHC...\n");
            ehci_startHC(ehci);
            puts("ehci_startHC done\n");

            puts("Supports port indicator: ");

            if(capRegs->HCSPARAMS.P_INDICATOR)
                puts("Yes\n");
            else
                puts("No\n");

            puts("Supports port power control: ");

            if(capRegs->HCSPARAMS.PPC)
                puts("Yes\n");
            else
                puts("No\n");

            uint8_t N_PORTS = capRegs->HCSPARAMS.N_PORTS;
            printf("N_PORTS: %u\n", N_PORTS);

            ehci_checkPorts(ehci);
        } else {
            puts("EHCI BAR0 Memory could not be allocated\n");
        }
    }
}

void ehci_initHC(ehci_t* ehci) {
    uint16_t pciCommandReg = pci_configReadWord(ehci->pciDev->bus, ehci->pciDev->device, ehci->pciDev->func, PCI_COMMAND);
    pci_configWriteWord(ehci->pciDev->bus, ehci->pciDev->device, ehci->pciDev->func, PCI_COMMAND, pciCommandReg | 0x0002 | 0x0004);

    irq_install_handler_arg(ehci->pciDev->irq, ehci_handler, ehci);
}

#pragma GCC push_options
#pragma GCC optimize ("O0")

void ehci_resetHC(ehci_t* ehci) {
    ehci_USBCMD_t usbcmd = ehci->opRegs->USBCMD;
    usbcmd.RS = false;
    ehci->opRegs->USBCMD = usbcmd;

    // Software should not set the HCRESET bit to a one when the HCHalted bit in the USBSTS register is a zero.
    // Attempting to reset an actively running host controller will result in undefined behavior.
    ehci_USBSTS_t usbsts = ehci->opRegs->USBSTS;

    while(usbsts.HCHalted == 0) {
        sleepMilliSeconds(10);
        usbsts = ehci->opRegs->USBSTS;
    }

    usbcmd = ehci->opRegs->USBCMD;
    usbcmd.HCRESET = true;
    ehci->opRegs->USBCMD = usbcmd;

    // Wait while HCReset bit is still set
    int8_t timeout = 30;

    while(ehci->opRegs->USBCMD.HCRESET) {
        sleepMilliSeconds(10);

        if(timeout == 0) {
            puts("ehci_resetHC timed out\n");
            break;
        }

        --timeout;
    }
}

void ehci_startHC(ehci_t* ehci) {
    ehci_disableLegacySupport(ehci);

    // Use 32 bit addressing
    ehci->opRegs->CTRLDSSEGMENT = 0;

    // Clear the USBSTS register
    ehci_USBSTS_t usbsts = {0};
    usbsts.USBINT = true;
    usbsts.USBERRINT = true;
    usbsts.port_change_detect = true;
    usbsts.frame_list_rollover = true;
    usbsts.host_system_error = true;
    usbsts.interrupt_on_async_advance = true;
    usbsts.HCHalted = true;
    ehci->opRegs->USBSTS = usbsts;

    // Determine which events should cause an interrupt
    ehci_USBINTR_t usbintr = {0};
    usbintr.port_change_interrupt_enable = true;
    usbintr.interrupt_on_async_advance_enable = true;
    usbintr.host_system_error_enable = true;
    usbintr.usb_error_interrupt_enable = true;
    usbintr.usb_interrupt_enable = true;
    ehci->opRegs->USBINTR = usbintr;

    // Set the Interrupt Threshold Control (min. time between interrupts) to 8 Microframes (1ms)
    ehci_USBCMD_t usbcmd = ehci->opRegs->USBCMD;
    usbcmd.interrupt_threshold_control = 0x08; // 8 micro-frames (1ms)
    ehci->opRegs->USBCMD = usbcmd;

    // Turn the host controller on
    usbsts = ehci->opRegs->USBSTS;

    if(usbsts.HCHalted) {
        usbcmd = ehci->opRegs->USBCMD;
        usbcmd.RS = true;
        ehci->opRegs->USBCMD = usbcmd;
    }

    // Set port routing to route all ports to the HC as opposed to companion HCs
    ehci->opRegs->CONFIGFLAG = 1;
}

#pragma GCC pop_options

void ehci_disableLegacySupport(ehci_t* ehci) {
    uint8_t eecp = ehci->capRegs->HCCPARAMS.EECP;

    if(eecp >= 0x40) {
        uint8_t eecp_id = 0;

        while(eecp) {
            eecp_id = pci_configReadByte(ehci->pciDev->bus, ehci->pciDev->device, ehci->pciDev->func, eecp);

            if(eecp_id == 1)
                break;

            eecp = pci_configReadByte(ehci->pciDev->bus, ehci->pciDev->device, ehci->pciDev->func, eecp + 1);
        }

        uint8_t BIOSownedSemaphore = eecp + 2; // R/W - only Bit 16 (Bit 23:17 Reserved, must be set to zero)
        uint8_t OSownedSemaphore = eecp + 3; // R/W - only Bit 24 (Bit 31:25 Reserved, must be set to zero)
        uint8_t USBLEGCTLSTS = eecp + 4; // USB Legacy Support Control/Status (DWORD, cf. EHCI 1.0 spec, 2.1.8)

        if(eecp_id == 1 && (pci_configReadByte(ehci->pciDev->bus, ehci->pciDev->device, ehci->pciDev->func, BIOSownedSemaphore) & 0x01)) {
            pci_configWriteWord(ehci->pciDev->bus, ehci->pciDev->device, ehci->pciDev->func, BIOSownedSemaphore, 0x01);

            int16_t timeout = 250;


            while((pci_configReadByte(ehci->pciDev->bus, ehci->pciDev->device, ehci->pciDev->func, BIOSownedSemaphore) & 0x01) && (timeout > 0)) {
                sleepMilliSeconds(10);
                --timeout;
            }

            while((pci_configReadByte(ehci->pciDev->bus, ehci->pciDev->device, ehci->pciDev->func, OSownedSemaphore) & 0x01) && (timeout > 0)) {
                sleepMilliSeconds(10);
                --timeout;
            }

            pci_configWriteDword(ehci->pciDev->bus, ehci->pciDev->device, ehci->pciDev->func, USBLEGCTLSTS, 0);
        } else {
            puts("ehci_disableLegacySupport BIOS did not own the EHCI. No action needed.\n");
        }
    } else {
        puts("ehci_disableLegacySupport no valid EECP found\n");
    }
}

#pragma GCC push_options
#pragma GCC optimize ("O0")

void ehci_checkPorts(ehci_t* ehci) {
    uint8_t N_PORTS = ehci->capRegs->HCSPARAMS.N_PORTS;

    for(uint8_t i = 0; i < N_PORTS; ++i) {
        ehci_PORTSC_t portsc = ehci->opRegs->PORTSC[i];

        if(portsc.connect_status_change) {
            ehci->opRegs->PORTSC[i] = portsc;

            if(portsc.current_connect_status)
                printf("EHCI Device on port %u connected\n", i);
            else
                printf("EHCI Device on port %u disconnected\n", i);
        }
    }
}

#pragma GCC pop_options

void ehci_handler(registers_t* r, void* arg) {
    ehci_t* ehci = arg;

    // Check if interrupt came from this EHCI
    ehci_USBSTS_t usbsts = ehci->opRegs->USBSTS;

    if(*((uint32_t*)(&usbsts)) == 0) {
        puts("EHCI interrupt ignored\n");
        return;
    }

    // Reset interrupt occured flag by writing the value back to the register
    // Section 2.3.2 of Intel EHCI Spec
    ehci->opRegs->USBSTS = usbsts;

    if(usbsts.USBERRINT) {
        puts("EHCI USB Error interrupt\n");
    }

    if(usbsts.port_change_detect) {
        ehci_checkPorts(ehci);
    }

    printf("USBSTS: %X\n", usbsts);
}
