#ifndef H_TASK1
#define H_TASK1

#include "syscall/syscall.h"

void task1() {
    write(0, "write syscall (from task1)\n");
    syscall0(SYSCALL_TEST);

    syscall0(SYSCALL_SCHED_YIELD);
    // while(1){}
}

#endif // H_TASK1
