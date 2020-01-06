#include "os.h"
#include "task.h"

/* Array of function pointers handling custom IRQ handlers for a given IRQ */
void* irq_routines[256];

/* Implement a custom IRQ handler for the given IRQ */
void irq_install_handler(int32_t ir, void (*handler)(registers_t* r)) {irq_routines[ir+32] = handler;}

/* Clear the custom IRQ handler */
void irq_uninstall_handler(int32_t ir) {irq_routines[ir] = 0;}

/* Message string corresponding to the exception number 0-31: exception_messages[interrupt_number] */
char* exception_messages[] =
{
    "Division By Zero",        "Debug",                         "Non Maskable Interrupt",    "Breakpoint",
    "Into Detected Overflow",  "Out of Bounds",                 "Invalid Opcode",            "No Coprocessor",
    "Double Fault",            "Coprocessor Segment Overrun",   "Bad TSS",                   "Segment Not Present",
    "Stack Fault",             "General Protection Fault",      "Page Fault",                "Unknown Interrupt",
    "Coprocessor Fault",       "Alignment Check",               "Machine Check",             "Reserved",
    "Reserved",                "Reserved",                      "Reserved",                  "Reserved",
    "Reserved",                "Reserved",                      "Reserved",                  "Reserved",
    "Reserved",                "Reserved",                      "Reserved",                  "Reserved"
};

registers_t* irq_handler(registers_t* r) {
    if (r->int_no < 32) { // exception
        if (r->int_no == 7) {
            NM_fxsr(r);
            return r;
        }

        settextcolor(4,0);

        if (r->int_no == 6 || r->int_no == 1) { // Invalid Opcode
            printf("err_code: %X address(eip): %X\n", r->err_code, r->eip);
            printf("edi: %X esi: %X ebp: %X eax: %X ebx: %X ecx: %X edx: %X\n", r->edi, r->esi, r->ebp, r->eax, r->ebx, r->ecx, r->edx);
            printf("cs: %X ds: %X es: %X fs: %X gs %X ss %X\n", r->cs, r->ds, r->es, r->fs, r->gs, r->ss);
            printf("int_no %d eflags %X useresp %X\n", r->int_no, r->eflags, r->useresp);
        }

        if (r->int_no == 14) { // Page Fault
            uint32_t faulting_address;
            __asm__ volatile("mov %%cr2, %0" : "=r" (faulting_address)); // faulting address <== CR2 register

            // The error code gives us details of what happened.
            int32_t present   = !(r->err_code & 0x1); // Page not present
            int32_t rw        =   r->err_code & 0x2;  // Write operation?
            int32_t us        =   r->err_code & 0x4;  // Processor was in user-mode?
            int32_t reserved  =   r->err_code & 0x8;  // Overwritten CPU-reserved bits of page entry?
            int32_t id        =   r->err_code & 0x10; // Caused by an instruction fetch?

            // Output an error message.
                          puts("\nPage Fault (");
            if (present)  puts("page not present");
            if (rw)       puts(" read-only - write operation");
            if (us)       puts(" user-mode");
            if (reserved) puts(" overwritten CPU-reserved bits of page entry");
            if (id)       puts(" caused by an instruction fetch");
                          printf(") at %X - EIP: %X\n", faulting_address, r->eip);
        }

        printf("err_code: %X address(eip): %X\n", r->err_code, r->eip);
        printf("edi: %X esi: %X ebp: %X eax: %X ebx: %X ecx: %X edx: %X\n", r->edi, r->esi, r->ebp, r->eax, r->ebx, r->ecx, r->edx);
        printf("cs: %X ds: %X es: %X fs: %X gs %X ss %X\n", r->cs, r->ds, r->es, r->fs, r->gs, r->ss);
        printf("int_no %d eflags %X useresp %X\n", r->int_no, r->eflags, r->useresp);
        uint16_t fpu_status_word;
        __asm__ volatile("fnstsw %0": "=m"(fpu_status_word));
        printf("fpu_status_word: %x\n", fpu_status_word);
        printf("esp: %X\n", r);
        printf("\n%s >>> Exception <<<", exception_messages[r->int_no]);

        exitCurrentTask();
        return r;
    }

    if(ODA.ts_flag && (r->int_no == 0x20 || r->int_no == 0x7E))
        r = (registers_t*) task_switch((uint32_t) r); //new task's esp

    void (*handler)(registers_t* r);
    handler = irq_routines[r->int_no];

    if(handler)
        handler(r);

    if(r->int_no >= 40)
        outportb(0xA0, 0x20);
    outportb(0x20, 0x20);

    return r;
}
