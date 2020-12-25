#include "descriptor_tables.h"
#include "os.h"

#define NUMBER_GDT_GATES 6  // 0-5: Null, Kernel Code, Kernel Data, User Code, User Data, TSS

// Global Descriptor Table
gdt_entry_t gdt[NUMBER_GDT_GATES];

// GDT Descriptor
gdt_ptr_t gdt_register;

/**
 * @brief Setup a descriptor in the Global Descriptor Table.
 * 
 * @param num index of the segment
 * @param base address to the beginning of the segment
 * @param limit number of the maximum addressable unit (20 bit value) (either in bytes, or in pages)
 * @param access access byte
 * @param gran flags (granularity bit and size bit)
 */
void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    // Setup the descriptor base address
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;

    // Setup the descriptor limits
    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);

    // Finally, set up the granularity and access flags
    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access      = access;
}

void gdt_install() {
    // Setup the GDT pointer and limit
    gdt_register.base = (uint32_t)&gdt;
    gdt_register.limit = (sizeof(gdt_entry_t) * NUMBER_GDT_GATES) - 1;

    // GDT GATES - desriptors with pointers to the linear memory address
    gdt_set_gate(0, 0, 0, 0, 0);                 // NULL descriptor
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);  // CODE, privilege level 0 for kernel mode (supervisor)
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);  // DATA, privilege level 0 for kernel mode (supervisor)
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);  // CODE, privilege level 3 for user mode
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);  // DATA, privilege level 3 for user mode

    write_tss(5, 0x10, 0x0);  // num, ss0, esp0

    gdt_flush((uint32_t)&gdt_register);
    tss_flush();
}
