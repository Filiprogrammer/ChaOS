#include "descriptor_tables.h"

gdt_entry_t gdt_entries[6];
gdt_ptr_t gdt_ptr;
idt_entry_t idt_entries[256];
idt_ptr_t idt_ptr;
tss_entry_t tss;

/**
 * @brief Initialize the Task State Segment structure.
 * 
 * @param num index of the segment in the Global Descriptor Table
 * @param ss0 kernel stack segment
 * @param esp0 kernel stack pointer
 */
void write_tss(int32_t num, uint16_t ss0, uint32_t esp0) {
    // Firstly, let's compute the base and limit of our entry into the GDT.
    uint32_t base = (uint32_t)&tss;
    uint32_t limit = sizeof(tss);

    // Now, add our TSS descriptor's address to the GDT.
    gdt_set_gate(num, base, limit, 0xE9, 0x00);

    // Ensure the descriptor is initially zero.
    memset(&tss, 0, sizeof(tss));

    tss.ss0 = ss0;    // Set the kernel stack segment.
    tss.esp0 = esp0;  // Set the kernel stack pointer.

    tss.cs = 0x08;
    tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x10;
}

/**
 * @brief Set the kernel stack in the TSS.
 * 
 * @param stack kernel stack address
 */
void set_kernel_stack(uint32_t stack) {
    tss.esp0 = stack;
}
