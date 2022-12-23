#include "syscall.h"
#include "proc/scheduler.h"

static void write_syscall(uint64_t fd, const char* buffer) {
    kset_color(BLACK, WHITE);

    if (fd == 0) // stdout
        kprintf("%s", buffer);
    else
        kprintln("only stdout is supported so far");
}

static void test_syscall(void) {
    kset_color(BLACK, WHITE);
    kprintln("test syscall");
}

static void sched_yield_syscall(void) {
    kset_color(BLACK, WHITE);
    kprintln("sched_yield syscall");
    switch_task();
}

uint64_t syscall_table[SYSCALLS_COUNT] = {
    [SYSCALL_WRITE]         = (uint64_t) &write_syscall,
    [SYSCALL_TEST]          = (uint64_t) &test_syscall,
    [SYSCALL_SCHED_YIELD]   = (uint64_t) &sched_yield_syscall,
};
