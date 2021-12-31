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
    kprintln("func_addr:  %x", s_tasks[0].user_func);
    kprintln("pml4_addr:  %x", s_tasks[0].virt_addr_space);
    kprintln("stack_addr: %x", s_tasks[0].user_stack);

    if (s_tasks[0].user_func == 0x0 || s_tasks[0].user_stack == 0x0 || s_tasks[0].virt_addr_space == 0x0) {
        kprintln("Error no init task set");
        return;
    }

    kprintln("going into user mode...");
    jump_usermode(s_tasks[0].user_stack, s_tasks[0].user_func, s_tasks[0].virt_addr_space);
}

// only one task so far so instead just idle
void switch_task() {
    kprintln("next task... (no other task so idle)");
    // TODO: syscalls in syscalls don't work
    // syscall(SYSCALL_WRITE);

    while(1){};
}
