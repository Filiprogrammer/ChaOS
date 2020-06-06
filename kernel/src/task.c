#include "task.h"

#include "fileManager.h"
#include "kheap.h"
#include "math.h"
#include "paging.h"
#include "queue.h"

#define QUEUE_NUMBER 4

static queue_t queues[QUEUE_NUMBER] = {{NULL, NULL}, {NULL, NULL}, {NULL, NULL}, {NULL, NULL}};
static queue_t queues_sleeping[QUEUE_NUMBER] = {{NULL, NULL}, {NULL, NULL}, {NULL, NULL}, {NULL, NULL}};

// If true timemillis will not be updated
bool timefreeze = false;

uint32_t timemillis;

uint8_t current_queue = 0;
uint8_t timeQuantumCounter = 0;

task_t* kernel_task;

task_t* doNothing_task;

// The currently running task.
task_t* current_task;

volatile task_t* FPUTask;

// Some externs are needed
extern tss_entry_t tss;
extern void irq_tail();

uint32_t next_pid = 1;  // The next available process ID.

/**
 * @brief Get the Process ID of the current task.
 * 
 * @return int32_t the process ID
 */
int32_t getpid() {
    return current_task->id;
}

static page_directory_t* const KERNEL_DIRECTORY = NULL;

static task_t* _create_task(page_directory_t* directory, void* entry, uint8_t privilege, int8_t priority);

static void doNothing() {
    for (;;)
        hlt();
}

#ifdef _DIAGNOSIS_
static void doLogging() {
    for (;;) {
        sleepCurrentTask(5000);

        ODA.ts_flag = false;
        log_task_list();
        printf("CPU Frequency: %uMHz\n", ((uint32_t)ODA.cpu_frequency) / 1000000);  // TODO: Print out 64 bit value
        ODA.ts_flag = true;
    }
}
#endif

void tasking_install() {
    cli();

#ifdef _DIAGNOSIS_
    settextcolor(2, 0);
    puts("1st_task: ");
    settextcolor(15, 0);
#endif

    current_task = kernel_task = (task_t*)malloc(sizeof(task_t), 0);  // first task (kernel task)
    current_task->id = next_pid++;
    current_task->esp = current_task->ebp = 0;
    current_task->eip = 0;
    current_task->page_directory = KERNEL_DIRECTORY;
    current_task->timeout = 0;
    current_task->last_active = rdtsc();
    current_task->cpu_time_used = 0;
    current_task->FPUPtr = 0;
    current_task->priority = 0;
    current_task->nice = 0;

#ifdef _DIAGNOSIS_
    settextcolor(2, 0);
    puts("1st_ks: ");
    settextcolor(15, 0);
#endif

    current_task->kernel_stack = (uint32_t)malloc(KERNEL_STACK_SIZE, PAGESIZE) + KERNEL_STACK_SIZE;

    page_directory_t* pd = KERNEL_DIRECTORY;
    doNothing_task = _create_task(pd, &doNothing, 0, -5);

#ifdef _DIAGNOSIS_
    create_task(pd, &doLogging, 0, -5);
#endif

    sti();
}

/**
 * @brief Create a task and add it to the highest priority queue.
 * 
 * @param directory page directory
 * @param entry entrypoint
 * @param privilege ring the task should run on (3 for userland)
 * @param priority task priority
 * @return task_t* pointer to the new task strcuture
 */
task_t* create_task(page_directory_t* directory, void* entry, uint8_t privilege, int8_t priority) {
    cli();
    task_t* new_task = _create_task(directory, entry, privilege, priority);
    queue_enqueue(&(queues[0]), new_task);
    sti();

    return new_task;
}

static task_t* _create_task(page_directory_t* directory, void* entry, uint8_t privilege, int8_t priority) {
    task_t* new_task = (task_t*)malloc(sizeof(task_t), 0);
    new_task->id = next_pid++;
    new_task->page_directory = directory;

    new_task->kernel_stack = (uint32_t)malloc(KERNEL_STACK_SIZE, PAGESIZE) + KERNEL_STACK_SIZE;

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

    if (privilege == 3)
        data_segment = 0x23;  // 0x20|0x3=0x23

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
    new_task->nice = 0;

    return new_task;
}

void exit_task(task_t* t) {
    ODA.ts_flag = false;

    if (t == kernel_task) {
        puts("\nSystem Halted!");
        for (;;)
            hlt();
    }

    for (uint8_t i = 0; i < QUEUE_NUMBER; ++i)
        if (queue_removeElement(&(queues[i]), t))
            goto task_dequeued;

    for (uint8_t i = 0; i < QUEUE_NUMBER; ++i)
        if (queue_removeElement(&(queues_sleeping[i]), t))
            goto task_dequeued;

    task_dequeued:

    if (t->FPUPtr)
        free(t->FPUPtr);

    if (FPUTask == t)
        FPUTask = NULL;

    void* pkernelstack = (void*)(t->kernel_stack - KERNEL_STACK_SIZE);
    free(pkernelstack);
    paging_destroyPageDirectory(t->page_directory);
    printf("\nTask %d exited!\n", t->id);

    ODA.ts_flag = true;

    free(t);

    if (t == current_task) {
        current_task = NULL;
        switch_context();
    }
}

/**
 * @brief Kill the currently running task.
 * 
 */
void exitCurrentTask() {
    exit_task((task_t*)current_task);
}

void NM_fxsr(registers_t* r) {
    __asm__ volatile("clts");  // CLearTS: reset the TS bit (no. 3) in CR0 to disable #NM

    // save FPU data
    if (FPUTask)  // fxsave to FPUTask->FPUptr
        __asm__ volatile("fxsave (%0)" ::"r"(FPUTask->FPUPtr));

    FPUTask = current_task;  // store the last task using FPU

    // restore FPU data
    if (current_task->FPUPtr)  // fxrstor from current_task->FPUptr
        __asm__ volatile("fxrstor (%0)" ::"r"(current_task->FPUPtr));
    else
        current_task->FPUPtr = malloc(512, 16);
}

uint32_t task_switch(uint32_t esp) {
    if (!timefreeze)
        timemillis = timer_getMilliseconds();

    ODA.ts_flag = false;

    // Move sleeping tasks that are done sleeping back to the normal queues
    for (uint8_t i = 0; i < QUEUE_NUMBER; ++i) {
        queue_t* queue_sleeping = &(queues_sleeping[i]);

        if (!queue_isEmpty(queue_sleeping)) {
            task_t* sleeping_task = queue_peek(queue_sleeping, 0);
            uint32_t j = 1;

            while (sleeping_task != NULL) {
                if (sleeping_task->timeout <= timemillis) {
                    queue_removeElement(queue_sleeping, sleeping_task);

                    if (sleeping_task->nice > 50 * (QUEUE_NUMBER - i))
                        // Promote task to a higher priority queue because it was a good boy
                        queue_enqueue(&(queues[MAX(i - 1, 0)]), sleeping_task);
                    else
                        queue_enqueue(&(queues[i]), sleeping_task);
                }

                sleeping_task = queue_peek(queue_sleeping, j);
                ++j;
            }
        }
    }

    task_t* old_task = (task_t*)current_task;

    if (old_task != NULL) {
        old_task->esp = esp;  // save esp

        if (old_task != doNothing_task) {
            ++timeQuantumCounter;

            if (timemillis < old_task->timeout) {
                // Task was faster than the time quantum
                old_task->nice = (4 * old_task->nice) / 5 + (old_task->timeout - timemillis) / 5;
            } else if (timeQuantumCounter > current_queue) {
                // Task took too long, demoting it to a lower priority queue
                queue_enqueue(&(queues[MIN(current_queue + 1, QUEUE_NUMBER - 1)]), old_task);
                old_task->nice = 0;
            } else {
                // Task still has some time left to run
                tss.esp = old_task->esp;
                tss.esp0 = old_task->kernel_stack;
                tss.ebp = old_task->ebp;
                tss.ss = old_task->ss;

                ODA.ts_flag = true;
                timefreeze = false;
                return old_task->esp;
            }

            uint64_t cpuCycles = rdtsc() - old_task->last_active;
            uint32_t microSeconds = cpuCyclesToMicroSeconds(cpuCycles);
            old_task->cpu_time_used += microSeconds;
        }
    }

    timeQuantumCounter = 0;

    // Find the next task to run
    for (uint8_t i = 0; i < QUEUE_NUMBER; ++i) {
        queue_t* queue = &(queues[i]);

        if (!queue_isEmpty(queue)) {
            current_task = queue_dequeue(queue);
            current_queue = i;
            goto found_new_task;
        }
    }

    current_task = doNothing_task;

    found_new_task:

    current_task->last_active = rdtsc();

    // new_task
    paging_switch(current_task->page_directory);
    tss.esp = current_task->esp;
    tss.esp0 = current_task->kernel_stack;
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
    timefreeze = false;
    return current_task->esp;  // return new task's esp
}

/**
 * @brief Make the current task sleep for the given time.
 * 
 * @param ms time to sleep in milliseconds
 */
void sleepCurrentTask(uint32_t ms) {
    if (ms > 0) {
        cli();
        timefreeze = true;
        timemillis = timer_getMilliseconds();
        current_task->timeout = timemillis + ms;
        queue_enqueue(&(queues_sleeping[current_queue]), current_task);
        sti();
    }

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
    settextcolor(4, 0);
    printf("Running on Q%u: \n", current_queue);
    task_log(current_task);

    for (uint8_t k = 0; k < 2; ++k) {
        queue_t* _queues;
        char* str;

        if (k == 0) {
            _queues = &(queues[0]);
            str = "Q%u:\n";
        } else {
            _queues = &(queues_sleeping[0]);
            str = "Sleeping Q%u:\n";
        }

        for (uint8_t i = 0; i < QUEUE_NUMBER; ++i) {
            settextcolor(3, 0);
            printf(str, i);

            queue_t* queue = &(_queues[i]);

            if (!queue_isEmpty(queue)) {
                task_t* task = queue_peek(queue, 0);
                uint32_t j = 1;

                while (task != NULL) {
                    task_log(task);
                    task = queue_peek(queue, j);
                    ++j;
                }
            }
        }
    }
}

void task_log(task_t* t) {
    settextcolor(5, 0);
    printf("\nid: %d ", t->id);               // Process ID
    printf("ebp: %X ", t->ebp);               // Base pointer
    printf("esp: %X ", t->esp);               // Stack pointer
    printf("eip: %X ", t->eip);               // Instruction pointer
    printf("PD: %X ", t->page_directory);     // Page directory.
    printf("k_stack: %X ", t->kernel_stack);  // Kernel stack location.
    printf("timeout: %u ", t->timeout);
    printf("nice: %u ", t->nice);
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
