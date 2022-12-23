#ifndef H_TASK_SIMPLE
#define H_TASK_SIMPLE 

#include "syscall/syscall.h"

void task1(void) {
    write(0, "write syscall (from task1)\n");
    syscall0(SYSCALL_TEST);

    sched_yield();
}

void task2(void) {
    write(0, "from task2: write syscall\n");
    syscall0(SYSCALL_TEST);

    sched_yield();
}

#endif // H_TASK_SIMPLE
