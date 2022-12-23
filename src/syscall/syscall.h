#ifndef H_SYSCALL
#define H_SYSCALL

#include "types.h"
#include "io/io.h"

typedef enum {
    SYSCALL_WRITE = 0,
    SYSCALL_TEST,
    SYSCALL_SCHED_YIELD,

    SYSCALLS_COUNT
} syscall_num;


static inline void syscall0(uint64_t num) {
    if (num >= SYSCALLS_COUNT)
        return;
    // rax = syscall number
    __asm__ volatile ("syscall" : : "a"(num) : "rcx", "r11");
    // rcx in the clobber list very important!!!
    // otherwise gcc screws up with setting rax correctly (when optimization is enabled),
    // because gcc doesn't know rcx will be set on syscall
    // see Doc/objdump(1 - 3)
}
static inline void syscall1(uint64_t num, uint64_t arg1) {
    if (num >= SYSCALLS_COUNT)
        return;
    __asm__ volatile ("syscall" : : "a"(num), "D"(arg1) : "rcx", "r11");
}

static inline void syscall2(uint64_t num, uint64_t arg1, uint64_t arg2) {
    if (num >= SYSCALLS_COUNT)
        return;
    __asm__ volatile ("syscall" : : "a"(num), "D"(arg1), "S"(arg2) : "rcx", "r11");
}

static inline void syscall3(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
    if (num >= SYSCALLS_COUNT)
        return;
    __asm__ volatile ("syscall" : : "a"(num), "D"(arg1), "S"(arg2), "d"(arg3): "rcx", "r11");
}

static inline void syscall4(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
    if (num >= SYSCALLS_COUNT)
        return;

    __asm__ volatile (
        "mov %4, %%r10\n"
        "syscall"
        :
        : "a"(num), "D"(arg1), "S"(arg2), "d"(arg3), "r"(arg4)
        : "rcx", "r10", "r11"
    );
}


static inline void write(uint64_t fd, const char* buffer) {
    syscall2(SYSCALL_WRITE, fd, (uint64_t) buffer);
}

static inline void sched_yield(void) {
    syscall0(SYSCALL_SCHED_YIELD);
}

#endif // H_SYSCALL
