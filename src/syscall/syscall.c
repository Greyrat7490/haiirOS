#include "syscall.h"
#include "proc/scheduler.h"

// TODO: get rid of the attributes
//      many push and pop operations
__attribute__((no_caller_saved_registers))      // without this attribute the rip would be wrong on return (-> page_fault)
static void task_end_syscall() {
    set_color(BLACK, WHITE);
    println("task end syscall...");
}

__attribute__((no_caller_saved_registers))
static void write_syscall() {
    set_color(BLACK, WHITE);
    println("write syscall");
}

__attribute__((no_caller_saved_registers))
static void sched_yield_syscall() {
    set_color(BLACK, WHITE);
    println("sched_yield syscall");
    switch_task();
}

uint64_t syscall_table[SYSCALLS_COUNT] = {
    [SYSCALL_WRITE]         = (uint64_t) &write_syscall,
    [SYSCALL_SCHED_YIELD]   = (uint64_t) &sched_yield_syscall,
    [SYSCALL_TASK_END]      = (uint64_t) &task_end_syscall
};
