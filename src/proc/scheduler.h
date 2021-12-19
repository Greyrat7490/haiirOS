#ifndef H_SCHEDULER
#define H_SCHEDULER

#include "types.h"

typedef struct TaskControlBlock {
    const char* name;
    uint64_t user_stack;
    uint64_t user_func;
    uint64_t virt_addr_space;
    struct TaskControlBlock* next_task;
    int state;
} TCB_t;

void add_tcb(TCB_t task);
void start_scheduler();

#endif // H_SCHEDULER
