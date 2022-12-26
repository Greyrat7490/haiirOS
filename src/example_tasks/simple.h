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
    int* ptr = (int*)0x8000000000;
    void* addr = mmap(ptr, 8, PROT_READ | PROT_WRITE | PROT_EXEC, 0, 0, 0);
    if ((int64_t)addr == -1) {
        write(0, "mmap error\n");
    } else {
        write(0, "mmap successful\n");
    }

    int a = *ptr;
    printInt(a);

    *ptr = 64;
    a = *ptr;
    printInt(a);

    *(ptr+1) = -69;
    printInt(*(ptr+1));

    sched_yield();
}

#endif // H_TASK_SIMPLE
