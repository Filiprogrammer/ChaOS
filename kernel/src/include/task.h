#ifndef TASK_H
#define TASK_H

#include "os.h"
#include "paging.h"
#include "descriptor_tables.h"
#include "list.h"

#define KERNEL_STACK_SIZE 0x2000      // Use a 2kb kernel stack.

typedef struct task {
    int32_t id;                       // Process ID.
    page_directory_t* page_directory; // Page directory.
    uint64_t cpu_time_used;           // in microseconds
    uint64_t last_active;
    listHead_t* threads;
    uint32_t next_threadId;
} task_t;

typedef struct thread {
    int32_t id;                       // Thread ID.
    uint32_t esp;                     // Stack pointer.
    uint32_t ss;
    uint32_t kernel_stack;            // Kernel stack location.
    uint32_t stack_begin;
    uint32_t stack_end;
    uint8_t* FPUPtr;
    task_t* parent;
    uint32_t timeout;                 // Milliseconds until timeout ends (for task sleep)
    uint32_t nice;
    page_directory_t* page_directory;
} thread_t;

void tasking_install();
uint32_t task_switch(uint32_t esp);
void sleepCurrentThread(uint32_t ms);
int32_t getpid();
bool create_thread(void* entry);
task_t* create_task (page_directory_t* directory, void* entry, uint8_t privilege);
void exit_task(task_t* t);
void exitCurrentTask();
void exitCurrentThread();
void switch_context();

void task_log(task_t* t);
void TSS_log(tss_entry_t* tss);
void log_task_list();

void NM_fxsr(registers_t* r);

#endif
