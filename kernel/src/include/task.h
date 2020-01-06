#ifndef TASK_H
#define TASK_H

#include "os.h"
#include "paging.h"
#include "descriptor_tables.h"

#define KERNEL_STACK_SIZE 2048       // Use a 2kb kernel stack.

#define MAX_SLEEP_AVG 10000          // 10 ms

uint32_t initial_esp;

typedef struct task
{
    int32_t id;                       // Process ID.
    uint32_t esp, ebp;                // Stack and base pointers.
    uint32_t eip;                     // Instruction pointer.
    uint32_t ss;
    page_directory_t* page_directory; // Page directory.
    uint32_t kernel_stack;            // Kernel stack location.
    struct task* next;                // The next task in a linked list.
    uint32_t timeout;                 // Milliseconds until timeout ends (for task sleep)
    uint8_t* FPUPtr;
    uint64_t cpu_time_used;           // in microseconds
    uint64_t last_active;
    int8_t priority;                  // from -5 to 5
    uint16_t sleep_avg;               // in microseconds
    bool running;
} task_t;

void tasking_install();
uint32_t task_switch(uint32_t esp);
void sleepCurrentTask(uint32_t ms);
int32_t getpid();
task_t* create_task (page_directory_t* directory, void* entry, uint8_t privilege, int8_t priority);
void exit_task(task_t* t);
void exitCurrentTask();
void switch_context();

void task_log(task_t* t);
void TSS_log(tss_entry_t* tss);
void log_task_list();

void NM_fxsr(registers_t* r);

#endif
