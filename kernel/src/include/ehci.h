#ifndef EHCI_H
#define EHCI_H

#include "stdint.h"
#include "pci.h"

typedef struct {
    uint8_t N_PORTS : 4;
    bool PPC : 1;
    uint8_t _reserved0 : 2;
    bool port_routing_rules : 1;
    uint8_t N_PCC : 4;
    uint8_t N_CC : 4;
    bool P_INDICATOR : 1;
    uint8_t _reserved1 : 3;
    uint8_t debug_port_number : 4;
    uint8_t _reserved2;
} __attribute__((packed)) ehci_HCSPARAMS_t;

typedef struct {
    uint32_t addr_64bit : 1;
    uint32_t programable_frame_list_flag : 1;
    uint32_t asynchronous_Schedule_park_capability : 1;
    uint32_t _reserved0 : 1;
    uint32_t isochronous_scheduling_threshold : 4;
    uint32_t EECP : 8;
    uint32_t hardware_prefetch_capability : 1;
    uint32_t link_power_management_capability : 1;
    uint32_t per_port_change_event_capability : 1;
    uint32_t _32_frame_periodic_list_capability : 1;
    uint32_t _reserved1 : 12;
} __attribute__((packed)) ehci_HCCPARAMS_t;

typedef struct {
    uint8_t CAPLENGTH;      // Capability Register Length
    uint8_t reserved;
    uint16_t HCIVERSION;    // Interface Version Number (BCD)
    ehci_HCSPARAMS_t HCSPARAMS;     // Structural Parameters
    ehci_HCCPARAMS_t HCCPARAMS;     // Capability Parameters
    uint64_t HCSPPORTROUTE; // Companion Port Route Description
} __attribute__((packed)) ehci_capRegs_t;

typedef struct {
    uint32_t RS : 1;
    uint32_t HCRESET : 1;
    uint32_t frame_list_size : 2;
    uint32_t periodic_schedule_enable : 1;
    uint32_t asynchronous_schedule_enable : 1;
    uint32_t interrupt_on_async_advance_doorbell : 1;
    uint32_t light_host_controller_reset : 1;
    uint32_t asynchronous_schedule_park_mode_count : 2;
    uint32_t _reserved0 : 1;
    uint32_t asynchronous_schedule_park_mode_enable : 1;
    uint32_t periodic_schedule_prefetch_enable : 1;
    uint32_t asynchronous_schedule_prefetch_enable : 1;
    uint32_t fully_synchronized_prefetch : 1;
    uint32_t per_port_change_events_enable : 1;
    uint32_t interrupt_threshold_control : 8;
    uint32_t host_initiated_resume_duration : 4;
    uint32_t _reserved2 : 4;
} __attribute__((packed)) ehci_USBCMD_t;

typedef struct {
    uint32_t USBINT : 1;
    uint32_t USBERRINT : 1;
    uint32_t port_change_detect : 1;
    uint32_t frame_list_rollover : 1;
    uint32_t host_system_error : 1;
    uint32_t interrupt_on_async_advance : 1;
    uint32_t _reserved0 : 6;
    uint32_t HCHalted : 1;
    uint32_t reclamation : 1;
    uint32_t periodic_schedule_status : 1;
    uint32_t asynchronous_schedule_status : 1;
    uint32_t port_1 : 1;
    uint32_t port_2 : 1;
    uint32_t port_3 : 1;
    uint32_t port_4 : 1;
    uint32_t port_5 : 1;
    uint32_t port_6 : 1;
    uint32_t port_7 : 1;
    uint32_t port_8 : 1;
    uint32_t port_9 : 1;
    uint32_t port_10 : 1;
    uint32_t port_11 : 1;
    uint32_t port_12 : 1;
    uint32_t port_13 : 1;
    uint32_t port_14 : 1;
    uint32_t port_15 : 1;
    uint32_t port_16 : 1;
} __attribute__((packed)) ehci_USBSTS_t;

typedef struct {
    bool usb_interrupt_enable : 1;
    bool usb_error_interrupt_enable : 1;
    bool port_change_interrupt_enable : 1;
    bool frame_list_rollover_enable : 1;
    bool host_system_error_enable : 1;
    bool interrupt_on_async_advance_enable : 1;
    uint32_t _reserved : 26;
} __attribute__((packed)) ehci_USBINTR_t;

typedef struct {
    uint16_t frame_index : 14;
    uint32_t _reserved : 18;
} __attribute__((packed)) ehci_FRINDEX_t;

typedef struct {
    bool current_connect_status : 1;
    bool connect_status_change : 1;
    bool port_enabled : 1;
    bool port_enable_disable_change : 1;
    bool over_current_active : 1;
    bool over_current_change : 1;
    bool force_port_resume : 1;
    bool suspend : 1;
    bool port_reset : 1;
    bool _reserved0 : 1;
    uint8_t line_status : 2;
    bool port_power : 1;
    bool port_owner : 1;
    uint8_t port_indicator_control : 2;
    uint8_t port_test_control : 4;
    bool WKCNNT_E : 1;
    bool WKDSCNNT_E : 1;
    bool WKOC_E : 1;
    uint16_t _reserved1 : 9;
} __attribute__((packed)) ehci_PORTSC_t;

typedef struct {
    ehci_USBCMD_t USBCMD;           // USB Command
    ehci_USBSTS_t USBSTS;           // USB Status
    ehci_USBINTR_t USBINTR;          // USB Interrupt Enable
    ehci_FRINDEX_t FRINDEX;          // USB Frame Index
    uint32_t CTRLDSSEGMENT;    // 4G Segment Selector
    uint32_t PERIODICLISTBASE; // Frame List Base Address
    uint32_t ASYNCLISTADDR;    // Next Asynchronous List Address
    uint32_t reserved[9];
    uint32_t CONFIGFLAG;       // Configure Flag Register
    ehci_PORTSC_t PORTSC[16];       // Port Status/Control
} __attribute__((packed)) ehci_opRegs_t;

typedef struct {
    pciDev_t* pciDev;
    ehci_capRegs_t* capRegs;
    ehci_opRegs_t* opRegs;
} ehci_t;

void ehci_create(pciDev_t* pciDev);

#endif
