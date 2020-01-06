#include "task.h"
#include "kheap.h"
#include "paging.h"

// The currently running task.
volatile task_t* current_task;

// The start of the task linked list.
volatile task_t* ready_queue;

volatile task_t* FPUTask;

// Some externs are needed
extern tss_entry_t tss;
extern void irq_tail();

uint32_t next_pid = 1;  // The next available process ID.
int32_t getpid() { return current_task->id; }

static page_directory_t* const KERNEL_DIRECTORY = NULL;

static void doNothing() {
    //uint32_t i = 1;
    uint64_t LastRdtscValue = 0;
    uint32_t CurrentSeconds = 0xFFFFFFFF;
    for (;;) {
        //hlt();

        if (timer_getSeconds() != CurrentSeconds) {
            CurrentSeconds = timer_getSeconds();

            uint64_t Rdtsc = rdtsc();
            uint64_t RdtscDiff = Rdtsc - LastRdtscValue;
            uint32_t RdtscDiffHi = RdtscDiff >> 32;
            uint32_t RdtscDiffLo = RdtscDiff & 0xFFFFFFFF;
            LastRdtscValue = Rdtsc;

            if (!RdtscDiffHi) ODA.cpu_frequency = RdtscDiffLo;
            //printf("CPU Frequency: %uMHz\n", ((uint32_t)ODA.cpu_frequency)/1000000);
        }

        /*if((i % 512) == 0) {
            log_task_list();
        }
        ++i;*/

        //hlt();
        switch_context();
    }
}

void tasking_install() {
    cli();

#ifdef _DIAGNOSIS_
    settextcolor(2, 0);
    puts("1st_task: ");
    settextcolor(15, 0);
#endif

    current_task = ready_queue = (task_t*)malloc(sizeof(task_t), 0);  // first task (kernel task)
    current_task->id = next_pid++;
    current_task->esp = current_task->ebp = 0;
    current_task->eip = 0;
    current_task->page_directory = KERNEL_DIRECTORY;
    current_task->next = 0;
    current_task->timeout = 0;
    current_task->last_active = rdtsc();
    current_task->cpu_time_used = 0;
    current_task->FPUPtr = 0;
    current_task->priority = 0;
    current_task->running = true;

#ifdef _DIAGNOSIS_
    settextcolor(2, 0);
    puts("1st_ks: ");
    settextcolor(15, 0);
#endif

    current_task->kernel_stack = (uint32_t)malloc(KERNEL_STACK_SIZE, PAGESIZE) + KERNEL_STACK_SIZE;

    page_directory_t* pd = KERNEL_DIRECTORY;
    create_task(pd, &doNothing, 0, -5);
    sti();
}

task_t* create_task(page_directory_t* directory, void* entry, uint8_t privilege, int8_t priority) {
    cli();

#ifdef _DIAGNOSIS_
    settextcolor(2, 0);
    puts("cr_task: ");
    settextcolor(15, 0);
#endif

    task_t* new_task = (task_t*)malloc(sizeof(task_t), 0);
    new_task->id = next_pid++;
    new_task->page_directory = directory;

#ifdef _DIAGNOSIS_
    settextcolor(2, 0);
    puts("cr_task_ks: ");
    settextcolor(15, 0);
#endif

    new_task->kernel_stack = (uint32_t)malloc(KERNEL_STACK_SIZE, PAGESIZE) + KERNEL_STACK_SIZE;
    new_task->next = 0;

    task_t* tmp_task = (task_t*)ready_queue;
    while (tmp_task->next) {
        tmp_task = tmp_task->next;
    }
    tmp_task->next = new_task;

    uint32_t* kernel_stack = (uint32_t*)new_task->kernel_stack;

    uint32_t code_segment = 0x08, data_segment = 0x10;

///TEST///
    *(--kernel_stack) = 0x0;  // return address dummy
///TEST///

    if (privilege == 3) {
        // general information: Intel 3A Chapter 5.12
        *(--kernel_stack) = new_task->ss = 0x23;     // ss
        *(--kernel_stack) = new_task->kernel_stack;  // esp0
        code_segment = 0x1B;                         // 0x18|0x3=0x1B
    }

    *(--kernel_stack) = 0x0202;           // eflags = interrupts activated and iopl = 0
    *(--kernel_stack) = code_segment;     // cs
    *(--kernel_stack) = (uint32_t)entry;  // eip
    *(--kernel_stack) = 0;                // error code

    *(--kernel_stack) = 0;  // interrupt nummer

    // general purpose registers w/o esp
    *(--kernel_stack) = 0;
    *(--kernel_stack) = 0;
    *(--kernel_stack) = 0;
    *(--kernel_stack) = 0;
    *(--kernel_stack) = 0;
    *(--kernel_stack) = 0;
    *(--kernel_stack) = 0;

    if (privilege == 3) data_segment = 0x23;  // 0x20|0x3=0x23

    *(--kernel_stack) = data_segment;
    *(--kernel_stack) = data_segment;
    *(--kernel_stack) = data_segment;
    *(--kernel_stack) = data_segment;

    //setup TSS
    tss.ss0 = 0x10;
    tss.esp0 = new_task->kernel_stack;
    tss.ss = data_segment;

    //setup task_t
    new_task->ebp = 0xd00fc0de;  // test value
    new_task->esp = (uint32_t)kernel_stack;
    new_task->eip = (uint32_t)irq_tail;
    new_task->ss = data_segment;
    new_task->timeout = 0;
    new_task->last_active = rdtsc();
    new_task->cpu_time_used = 0;
    new_task->FPUPtr = 0;
    new_task->priority = priority;
    new_task->running = true;

    sti();

    return new_task;
}

void exit_task(task_t* t) {
    ODA.ts_flag = false;

    if (t == ready_queue) {
        puts("\nSystem Halted!");
        for (;;)
            hlt();
    }

    volatile task_t* tmp_task = ready_queue;
    do {
        if (tmp_task->next == t) {
            tmp_task->next = t->next;
            break;
        }
        if (tmp_task->next) {
            tmp_task = tmp_task->next;
        }
    } while (tmp_task->next);

    if (t->FPUPtr)
        free(t->FPUPtr);
    
    if (FPUTask == t)
        FPUTask = NULL;

    void* pkernelstack = (void*)(t->kernel_stack - KERNEL_STACK_SIZE);
    free(pkernelstack);
    paging_destroyPageDirectory(t->page_directory);
    t->running = false;
    printf("\nTask %d exited!\n", t->id);

    ODA.ts_flag = true;

    if (t == current_task)
        switch_context();
    else
        free(t);
}

void exitCurrentTask() {
    exit_task((task_t*)current_task);
}

void NM_fxsr(registers_t* r) {
    __asm__ volatile("clts");  // CLearTS: reset the TS bit (no. 3) in CR0 to disable #NM

    // save FPU data ...
    if (FPUTask)  // fxsave to FPUTask->FPUptr
        __asm__ volatile("fsave %0" ::"m"(FPUTask->FPUPtr));
        //__asm__ volatile("fxsave %0" :: "m" (*(uint8_t*)(FPUTask->fpu)));

    FPUTask = current_task;  // store the last task using FPU

    // restore FPU data ...
    if (current_task->FPUPtr)
        __asm__ volatile("frstor %0" ::"m"(current_task->FPUPtr));
        //__asm__ volatile("fxrstor %0" :: "m" (*(uint8_t*)(current_task->fpu)));
    else
        current_task->FPUPtr = malloc(108, 4);
}

uint32_t task_switch(uint32_t esp) {
    if (!current_task) return esp;
    ODA.ts_flag = false;
    task_t* old_task = (task_t*)current_task;
    old_task->esp = esp;  // save esp

    uint64_t cpuCycles = rdtsc() - old_task->last_active;
    uint32_t microSeconds = cpuCyclesToMicroSeconds(cpuCycles);
    old_task->cpu_time_used += microSeconds;

    // task switch
    current_task = old_task->next;

    if (!(old_task->running)) {
        free(old_task);
    }

    if (!current_task) current_task = ready_queue;

    if (old_task == current_task) {
        ODA.ts_flag = true;
        return esp;
    }

    while (timer_getMilliseconds() < current_task->timeout) {
        current_task = current_task->next;
        if (!current_task) current_task = ready_queue;
    }

    current_task->last_active = rdtsc();

    // new_task
    paging_switch(current_task->page_directory);
    tss.esp = current_task->esp;
    tss.esp0 = (current_task->kernel_stack);
    tss.ebp = current_task->ebp;
    tss.ss = current_task->ss;

    // Set TS
    if (current_task == FPUTask) {
        __asm__ volatile("clts");  // CLearTS: reset the TS bit (no. 3) in CR0 to disable #NM
    } else {
        uint32_t cr0;
        __asm__("mov %%cr0, %0" : "=r"(cr0)); // Read cr0
        cr0 |= 0x08;                          // Set the TS bit (no. 3) in CR0 to enable #NM (exception no. 7)
        __asm__("mov %0, %%cr0" ::"r"(cr0));  // Write cr0
    }

    ODA.ts_flag = true;
    return current_task->esp;  // return new task's esp
}

void sleepCurrentTask(uint32_t ms) {
    current_task->timeout = timer_getMilliseconds() + ms;
    switch_context();
}

/**
 * @brief Switch to next task.
 * 
 */
void switch_context() {
    __asm__ volatile("int $0x7E");
}

void log_task_list() {
    task_t* tmp_task = (task_t*)ready_queue;
    do {
        task_log(tmp_task);
        tmp_task = tmp_task->next;
    } while (tmp_task->next);
    task_log(tmp_task);
}

void task_log(task_t* t) {
    settextcolor(5, 0);
    printf("\nid: %d ", t->id);               // Process ID
    printf("ebp: %X ", t->ebp);               // Base pointer
    printf("esp: %X ", t->esp);               // Stack pointer
    printf("eip: %X ", t->eip);               // Instruction pointer
    printf("PD: %X ", t->page_directory);     // Page directory.
    printf("k_stack: %X ", t->kernel_stack);  // Kernel stack location.
    printf("next: %X ", t->next);             // The next task in a linked list.
    printf("timeout: %u ", t->timeout);
    printf("cpu_time_used: %ums\n", ((uint32_t)t->cpu_time_used) / 1000);
}

void TSS_log(tss_entry_t* tss) {
    settextcolor(6, 0);
    //printf("\nprev_tss: %x ", tss->prev_tss);
    printf("esp0: %x ", tss->esp0);
    printf("ss0: %x ", tss->ss0);
    //printf("esp1: %x ", tss->esp1);
    //printf("ss1: %x ", tss->ss1);
    //printf("esp2: %x ", tss->esp2);
    //printf("ss2: %x ", tss->ss2);
    printf("cr3: %x ", tss->cr3);
    printf("eip: %x ", tss->eip);
    printf("eflags: %x ", tss->eflags);
    printf("eax: %x ", tss->eax);
    printf("ecx: %x ", tss->ecx);
    printf("edx: %x ", tss->edx);
    printf("ebx: %x ", tss->ebx);
    printf("esp: %x ", tss->esp);
    printf("ebp: %x ", tss->ebp);
    printf("esi: %x ", tss->esi);
    printf("edi: %x ", tss->edi);
    printf("es: %x ", tss->es);
    printf("cs: %x ", tss->cs);
    printf("ss: %x ", tss->ss);
    printf("ds: %x ", tss->ds);
    printf("fs: %x ", tss->fs);
    printf("gs: %x ", tss->gs);
    //printf("ldt: %x ", tss->ldt);
    //printf("trap: %x ", tss->trap); //only 0 or 1
    //printf("iomap_base: %x ", tss->iomap_base);
}
