#ifndef H_TASK_SIMPLE
#define H_TASK_SIMPLE

#include "syscall/syscall.h"
#include "memory/Paging/hPaging.h"

void task1(void) {
    write(0, "write syscall (from task1)\n");

    sched_yield();
}

void task2(void) {
    write(0, "from task2: write syscall\n");

    // first address accessible for user (2nd pdp table)
    // maybe only first pd table for kernel
    int* ptr = (int*)0x8000000000;  

    mmap(ptr, 8, PROT_READ | PROT_WRITE | PROT_EXEC, 0, 0, 0);

    // TODO: fix in release virt addr is 0x0 (rdi is set in mmap to 0x0)
    volatile int a = *ptr;

    sched_yield();
}

#endif // H_TASK_SIMPLE
