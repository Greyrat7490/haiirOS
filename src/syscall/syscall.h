#ifndef H_SYSCALL
#define H_SYSCALL

#include "types.h"
#include "io/io.h"

typedef enum {
    SYSCALL_WRITE = 0,
    SYSCALL_SCHED_YIELD,
    SYSCALL_TASK_END,       // only tmp for testing

    SYSCALLS_COUNT
} syscall_num;

static inline void syscall(uint64_t num) {
    if (num >= SYSCALLS_COUNT)
        return;

    // rax = syscall number
    __asm__ volatile (
        "syscall"
        :
        : "a"(num)
        : "rcx", "r11"  // very important!!!
                        // otherwise gcc screws up with setting rax correctly (when optimization is enabled),
                        // because gcc doesn't know rcx was set after syscall
                        // see Doc/objdump(1 - 3)
    );
}

#endif // H_SYSCALL
