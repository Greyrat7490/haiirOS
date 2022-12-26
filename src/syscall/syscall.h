#ifndef H_SYSCALL
#define H_SYSCALL

#include "types.h"
#include "io/io.h"

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

    SYSCALLS_COUNT
} syscall_num;


static inline void syscall0(uint64_t num) {
    if (num >= SYSCALLS_COUNT)
        return;
    // important: rcx, r11 in clobber list
    // gcc doesn't know rcx, r11 will be set by syscall (see Doc/objdump1-3)
    // same for all other general purpose regs (therefore preserved in syscall_entry)
    __asm__ volatile ("syscall" : : "a"(num) : "rcx", "r11");
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

static inline void syscall6(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    if (num >= SYSCALLS_COUNT)
        return;

    __asm__ volatile (
        "mov %4, %%r10\n"
        "mov %5, %%r8\n"
        "mov %6, %%r9\n"
        "syscall"
        :
        : "a"(num), "D"(arg1), "S"(arg2), "d"(arg3), "r"(arg4), "r"(arg5), "r"(arg6)
        : "rcx", "r8", "r9", "r10", "r11"
    );
}


static inline void write(uint64_t fd, const char* buffer) {
    syscall2(SYS_WRITE, fd, (uint64_t) buffer);
}

static inline void sched_yield(void) {
    syscall0(SYS_SCHED_YIELD);
}

static inline void mmap(void* addr, uint64_t len, int prot, int flags, int fd, int64_t offset) {
    syscall6(SYS_MMAP, (uint64_t)addr, len, prot, flags, fd, offset);
}


#endif // H_SYSCALL
