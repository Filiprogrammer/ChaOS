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

// The currently running thread.
static thread_t* current_thread;

volatile thread_t* FPUThread;

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
    return current_thread->parent->id;
}

static page_directory_t* const KERNEL_DIRECTORY = NULL;

static task_t* _create_task(page_directory_t* directory);
static thread_t* _create_thread(task_t* task, void* entry, uint8_t privilege);

static void doNothing() {
    for (;;)
        hlt();
}

void tasking_install() {
    cli();
    kernel_task = _create_task(KERNEL_DIRECTORY);

    thread_t* thread = malloc(sizeof(thread_t), 0);
    thread->id = 0;
    thread->esp = 0;
    thread->ss = 0;
    thread->kernel_stack = (uint32_t)malloc(KERNEL_STACK_SIZE, PAGESIZE) + KERNEL_STACK_SIZE;
    thread->stack_begin = thread->kernel_stack - KERNEL_STACK_SIZE;
    thread->stack_end = thread->kernel_stack;
    thread->FPUPtr = NULL;
    thread->parent = kernel_task;
    thread->timeout = 0;
    thread->nice = 0;
    thread->page_directory = KERNEL_DIRECTORY;

    if (kernel_task->threads == NULL)
        kernel_task->threads = list_create();

    list_append(kernel_task->threads, thread);
    kernel_task->next_threadId = 1;
    current_thread = thread;

    doNothing_task = _create_task(KERNEL_DIRECTORY);
    _create_thread(doNothing_task, &doNothing, 0);

    sti();
}

static thread_t* _create_thread(task_t* task, void* entry, uint8_t privilege) {
    if (task->threads == NULL)
        task->threads = list_create();

    uint32_t stack_begin = 0x4F0000;
    uint32_t stack_size = 0x10000;

    if (privilege == 3) {
        size_t threadCount = list_getSize(task->threads);

        for (uint32_t i = 0; i < threadCount; ++i) {
            thread_t* thread = list_getElement(task->threads, i + 1);

            if ((stack_begin >= thread->stack_begin && stack_begin < thread->stack_end) ||
                (stack_begin + stack_size > thread->stack_begin && stack_begin + stack_size <= thread->stack_end)) {
                stack_begin = thread->stack_begin - stack_size;
            } else {
                break;
            }
        }

        // TODO: Handle the placement of the stack differently (probably place it before the .text and .data sections)
        // Allocate the stack
        if (!paging_allocVirt(task->page_directory, (void*)stack_begin, stack_size, MEM_USER | MEM_WRITABLE))
            return NULL;
    }

    thread_t* thread = malloc(sizeof(thread_t), 0);

    thread->kernel_stack = (uint32_t)malloc(KERNEL_STACK_SIZE, PAGESIZE) + KERNEL_STACK_SIZE;

    uint32_t* kernel_stack = (uint32_t*)thread->kernel_stack;

    uint32_t code_segment = 0x08, data_segment = 0x10;

    if (privilege == 0) {
        *(--kernel_stack) = (uint32_t)&exitCurrentThread;
        stack_begin = (uint32_t)kernel_stack;
        stack_size = KERNEL_STACK_SIZE;
    } else if (privilege == 3) {
        page_directory_t* active_pagedir = paging_getActivePageDirectory();
        paging_switch(task->page_directory);

        // Call the exitCurrentThread syscall at the end of the thread
        *((uint32_t*)(stack_begin + stack_size - 8)) = 0x001eb890;  // mov eax, 30
        *((uint32_t*)(stack_begin + stack_size - 4)) = 0x7fcd0000;  // int 0x7F
        *((uint32_t*)(stack_begin + stack_size - 12)) = stack_begin + stack_size - 7;

        paging_switch(active_pagedir);

        // general information: Intel 3A Chapter 5.12
        *(--kernel_stack) = thread->ss = 0x23;              // ss
        *(--kernel_stack) = stack_begin + stack_size - 12;  // esp0
        code_segment = 0x1B;                                // 0x18|0x3=0x1B
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

    thread->esp = (uint32_t)kernel_stack;
    thread->ss = data_segment;
    thread->stack_begin = stack_begin;
    thread->stack_end = stack_begin + stack_size;
    thread->FPUPtr = 0;
    thread->id = task->next_threadId++;
    thread->parent = task;
    thread->timeout = 0;
    thread->nice = 0;
    thread->page_directory = task->page_directory;

    list_append(task->threads, thread);

    return thread;
}

/**
 * @brief Create a thread in the current task and add it to the highest priority queue.
 * 
 * @param entry entrypoint
 * @return true thread was created successfully
 * @return false thread creation failed
 */
bool create_thread(void* entry) {
    if (current_thread == NULL)
        return false;

    task_t* task = current_thread->parent;

    uint8_t privilege = 3;

    if (task == kernel_task)
        privilege = 0;

    thread_t* thread = _create_thread(task, entry, privilege);

    if (thread == NULL)
        return false;

    cli();
    queue_enqueue(&(queues[0]), thread);
    sti();

    return true;
}

static task_t* _create_task(page_directory_t* directory) {
    task_t* new_task = (task_t*)malloc(sizeof(task_t), 0);
    new_task->id = next_pid++;
    new_task->page_directory = directory;
    new_task->next_threadId = 0;
    new_task->last_active = rdtsc();
    new_task->cpu_time_used = 0;
    new_task->threads = NULL;
    return new_task;
}

/**
 * @brief Create a task with one thread and add that thread to the highest priority queue.
 * 
 * @param directory page directory
 * @param entry entrypoint
 * @param privilege ring the task should run on (3 for userland)
 * @return task_t* pointer to the new task struct
 */
task_t* create_task(page_directory_t* directory, void* entry, uint8_t privilege) {
    task_t* new_task = _create_task(directory);
    thread_t* thread = _create_thread(new_task, entry, privilege);

    cli();
    queue_enqueue(&(queues[0]), thread);
    sti();

    return new_task;
}

void exit_task(task_t* task) {
    ODA.ts_flag = false;

    if (task == kernel_task) {
        puts("\nSystem Halted!");
        for (;;)
            hlt();
    }

    size_t threadCount = list_getSize(task->threads);

    for (uint32_t i = 0; i < threadCount; ++i) {
        thread_t* thread = list_getElement(task->threads, i + 1);

        if (thread == current_thread) {
            current_thread = NULL;
        } else {
            bool dequeued = false;

            for (uint8_t j = 0; j < QUEUE_NUMBER; ++j) {
                if (queue_removeElement(&(queues[j]), thread)) {
                    dequeued = true;
                    break;
                }
            }

            if (!dequeued) {
                for (uint8_t j = 0; j < QUEUE_NUMBER; ++j)
                    if (queue_removeElement(&(queues_sleeping[j]), thread))
                        break;
            }
        }

        if (FPUThread == thread)
            FPUThread = NULL;

        if (thread->FPUPtr)
            free(thread->FPUPtr);

        void* pkernelstack = (void*)(thread->kernel_stack - KERNEL_STACK_SIZE);
        free(pkernelstack);
        free(thread);
    }

    list_deleteAllWithoutData(task->threads);
    paging_destroyPageDirectory(task->page_directory);
    free(task);
    printf("\nTask %d exited!\n", task->id);

    ODA.ts_flag = true;

    if (current_thread == NULL)
        switch_context();
}

/**
 * @brief Kill the currently running task.
 * 
 */
void exitCurrentTask() {
    exit_task(current_thread->parent);
}

void exit_thread(thread_t* thread) {
    ODA.ts_flag = false;

    if (thread == list_getElement(kernel_task->threads, 1)) {
        puts("\nSystem Halted!");
        for (;;)
            hlt();
    }

    if (thread == current_thread) {
        current_thread = NULL;
    } else {
        bool dequeued = false;

        for (uint8_t j = 0; j < QUEUE_NUMBER; ++j) {
            if (queue_removeElement(&(queues[j]), thread)) {
                dequeued = true;
                break;
            }
        }

        if (!dequeued) {
            for (uint8_t j = 0; j < QUEUE_NUMBER; ++j)
                if (queue_removeElement(&(queues_sleeping[j]), thread))
                    break;
        }
    }

    if (FPUThread == thread)
        FPUThread = NULL;

    if (thread->FPUPtr)
        free(thread->FPUPtr);

    void* pkernelstack = (void*)(thread->kernel_stack - KERNEL_STACK_SIZE);
    free(pkernelstack);

    task_t* task = thread->parent;
    list_delete(task->threads, thread);
    printf("\nThread %d exited! (Task %d)\n", thread->id, task->id);
    free(thread);

    if (list_getSize(task->threads) == 0)
        exit_task(task);

    ODA.ts_flag = true;

    if (current_thread == NULL)
        switch_context();
}

void exitCurrentThread() {
    exit_thread(current_thread);
}

void NM_fxsr(registers_t* r) {
    __asm__ volatile("clts");  // CLearTS: reset the TS bit (no. 3) in CR0 to disable #NM

    // save FPU data
    if (FPUThread)  // fxsave to FPUTask->FPUptr
        __asm__ volatile("fxsave (%0)" ::"r"(FPUThread->FPUPtr));

    FPUThread = current_thread;  // store the last task using FPU

    // restore FPU data
    if (current_thread->FPUPtr)  // fxrstor from current_thread->FPUptr
        __asm__ volatile("fxrstor (%0)" ::"r"(current_thread->FPUPtr));
    else
        current_thread->FPUPtr = malloc(512, 16);
}

uint32_t task_switch(uint32_t esp) {
    if (!timefreeze)
        timemillis = timer_getMilliseconds();

    ODA.ts_flag = false;

    // Move sleeping threads that are done sleeping back to the normal queues
    for (uint8_t i = 0; i < QUEUE_NUMBER; ++i) {
        queue_t* queue_sleeping = &(queues_sleeping[i]);

        if (!queue_isEmpty(queue_sleeping)) {
            thread_t* sleeping_thread = queue_peek(queue_sleeping, 0);
            uint32_t j = 1;

            while (sleeping_thread != NULL) {
                if (sleeping_thread->timeout <= timemillis) {
                    queue_removeElement(queue_sleeping, sleeping_thread);

                    if (sleeping_thread->nice > 50 * (QUEUE_NUMBER - i))
                        // Promote thread to a higher priority queue because it was a good boy
                        queue_enqueue(&(queues[MAX(i - 1, 0)]), sleeping_thread);
                    else
                        queue_enqueue(&(queues[i]), sleeping_thread);
                }

                sleeping_thread = queue_peek(queue_sleeping, j);
                ++j;
            }
        }
    }

    thread_t* old_thread = current_thread;

    if (old_thread != NULL) {
        old_thread->esp = esp;  // save esp

        if (old_thread->parent != doNothing_task) {
            ++timeQuantumCounter;

            if (timemillis < old_thread->timeout) {
                // Task was faster than the time quantum
                old_thread->nice = (4 * old_thread->nice) / 5 + (old_thread->timeout - timemillis) / 5;
            } else if (timeQuantumCounter > current_queue) {
                // Task took too long, demoting it to a lower priority queue
                queue_enqueue(&(queues[MIN(current_queue + 1, QUEUE_NUMBER - 1)]), old_thread);
                old_thread->nice = 0;
            } else {
                // Task still has some time left to run
                ODA.ts_flag = true;
                timefreeze = false;
                return old_thread->esp;
            }

            uint64_t cpuCycles = rdtsc() - old_thread->parent->last_active;
            uint32_t microSeconds = cpuCyclesToMicroSeconds(cpuCycles);
            old_thread->parent->cpu_time_used += microSeconds;
            old_thread->page_directory = paging_getActivePageDirectory();
        }
    }

    timeQuantumCounter = 0;

    // Find the next task to run
    for (uint8_t i = 0; i < QUEUE_NUMBER; ++i) {
        queue_t* queue = &(queues[i]);

        if (!queue_isEmpty(queue)) {
            current_thread = queue_dequeue(queue);
            current_queue = i;
            goto found_new_task;
        }
    }

    // No thread to run found
    if (old_thread->parent == doNothing_task) {
        // doNothing task is already running, no need to switch to it
        ODA.ts_flag = true;
        timefreeze = false;
        return old_thread->esp;
    }

    current_thread = list_getElement(doNothing_task->threads, 1);

    found_new_task:

    current_thread->parent->last_active = rdtsc();

    // new_task
    paging_switch(current_thread->page_directory);
    tss.esp = current_thread->esp;
    tss.esp0 = current_thread->kernel_stack;
    tss.ss = current_thread->ss;

    // Set TS
    if (current_thread == FPUThread) {
        __asm__ volatile("clts");  // CLearTS: reset the TS bit (no. 3) in CR0 to disable #NM
    } else {
        uint32_t cr0;
        __asm__("mov %%cr0, %0" : "=r"(cr0)); // Read cr0
        cr0 |= 0x08;                          // Set the TS bit (no. 3) in CR0 to enable #NM (exception no. 7)
        __asm__("mov %0, %%cr0" ::"r"(cr0));  // Write cr0
    }

    ODA.ts_flag = true;
    timefreeze = false;
    return current_thread->esp;  // return new task's esp
}

/**
 * @brief Make the current thread sleep for the given time.
 * 
 * @param ms time to sleep in milliseconds
 */
void sleepCurrentThread(uint32_t ms) {
    if (ms > 0) {
        cli();
        timefreeze = true;
        timemillis = timer_getMilliseconds();
        current_thread->timeout = timemillis + ms;
        queue_enqueue(&(queues_sleeping[current_queue]), current_thread);
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
