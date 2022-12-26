#ifndef H_SYSCALL
#define H_SYSCALL

#include "types.h"

typedef enum {
    SYS_WRITE = 0,
    SYS_OPEN,
    SYS_READ,
    SYS_CLOSE,
    SYS_EXIT,
    SYS_SCHED_YIELD,
    SYS_MMAP,
    SYS_FORK,
    SYS_EXECVE,
    SYS_GETPID,
    SYS_KILL,
    SYS_GETCWD,
    SYS_MKDIR,
    SYS_RMDIR,
    SYS_RENAME,
    SYS_PAUSE,
    SYS_NANO_SLEEP,

    SYS_PRINT_INT,  // only tmp for debugging

    SYSCALLS_COUNT
} syscall_num;


static inline uint64_t syscall0(uint64_t num) {
    if (num >= SYSCALLS_COUNT)
        return -1;

    // important: rcx, r11 in clobber list
    // gcc doesn't know rcx, r11 will be set by syscall (see Doc/objdump1-3)
    // same for all other general purpose regs (therefore preserved in syscall_entry)
    uint64_t res;
    __asm__ volatile ("syscall" : "=a"(res) : "a"(num) : "rcx", "r11");
    return res;
}

static inline uint64_t syscall1(uint64_t num, uint64_t arg1) {
    if (num >= SYSCALLS_COUNT)
        return -1;

    uint64_t res;
    __asm__ volatile ("syscall" : "=a"(res) : "a"(num), "D"(arg1) : "rcx", "r11");
    return res;
}

static inline uint64_t syscall2(uint64_t num, uint64_t arg1, uint64_t arg2) {
    if (num >= SYSCALLS_COUNT)
        return -1;

    uint64_t res;
    __asm__ volatile ("syscall" : "=a"(res) : "a"(num), "D"(arg1), "S"(arg2) : "rcx", "r11");
    return res;
}

static inline uint64_t syscall3(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
    if (num >= SYSCALLS_COUNT)
        return -1;

    uint64_t res;
    __asm__ volatile ("syscall" : "=a"(res) : "a"(num), "D"(arg1), "S"(arg2), "d"(arg3): "rcx", "r11");
    return res;
}

static inline uint64_t syscall4(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
    if (num >= SYSCALLS_COUNT)
        return -1;

    uint64_t res;
    __asm__ volatile (
        "mov %4, %%r10\n"
        "syscall"
        : "=a"(res)
        : "a"(num), "D"(arg1), "S"(arg2), "d"(arg3), "r"(arg4)
        : "rcx", "r10", "r11"
    );
    return res;
}

static inline uint64_t syscall6(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    if (num >= SYSCALLS_COUNT)
        return -1;

    uint64_t res;
    __asm__ volatile (
        "mov %4, %%r10\n"
        "mov %5, %%r8\n"
        "mov %6, %%r9\n"
        "syscall"
        : "=a"(res)
        : "a"(num), "D"(arg1), "S"(arg2), "d"(arg3), "r"(arg4), "r"(arg5), "r"(arg6)
        : "rcx", "r8", "r9", "r10", "r11"
    );
    return res;
}


static inline uint64_t write(uint64_t fd, const char* buffer) {
    return syscall2(SYS_WRITE, fd, (uint64_t) buffer);
}

static inline uint64_t sched_yield(void) {
    return syscall0(SYS_SCHED_YIELD);
}

static inline void* mmap(void* addr, uint64_t len, int prot, int flags, int fd, int64_t offset) {
    return (void*)syscall6(SYS_MMAP, (uint64_t)addr, len, prot, flags, fd, offset);
}

static inline uint64_t printInt(int64_t i) {
    return syscall1(SYS_PRINT_INT, i);
}


#endif // H_SYSCALL
