#include "scheduler.h"
#include "proc/task.h"
#include "io/io.h"

extern void jump_usermode(uint64_t usr_stack_top, uint64_t usr_func_addr, uint64_t pml4_addr);

// for now only one task
static TCB_t s_tasks[1];


void add_tcb(TCB_t task) {
    s_tasks[0] = task;
}

void start_scheduler() {
    println("func_addr:  %x", s_tasks[0].user_func);
    println("pml4_addr:  %x", s_tasks[0].virt_addr_space);
    println("stack_addr: %x", s_tasks[0].user_stack);

    println("going into user mode...");
    jump_usermode(s_tasks[0].user_stack, s_tasks[0].user_func, s_tasks[0].virt_addr_space);

    println("scheduler end");

    while(1){}
}
