#include "syscall.h"
#include "time.h"
#include "io/io.h"
#include "proc/scheduler.h"
#include "memory/Paging/hPaging.h"

static uint64_t write_syscall(uint64_t fd, const char* buffer) {
    kset_color(BLACK, WHITE);

    if (fd == 0) { // stdout
        kprintf("%s", buffer);
        return 0;
    } else {
        kprintln("only stdout is supported so far");
        return -1;
    }
}

static int sched_yield_syscall(void) {
    switch_task();
    return 0;
}

static uint64_t open_syscall(const char* file, int flags, uint32_t mode) {
    (void)file;
    (void)flags;
    (void)mode;

    kset_color(BLACK, WHITE);
    kprintln("open syscall");
    return 0;
}

static uint64_t read_syscall(uint32_t fd, char* buffer, uint64_t count) {
    (void)fd;
    (void)buffer;
    (void)count;

    kset_color(BLACK, WHITE);
    kprintln("read syscall");
    return 0;
}

static uint64_t close_syscall(uint32_t fd) {
    (void)fd;

    kset_color(BLACK, WHITE);
    kprintln("close syscall");
    return 0;
}

static uint64_t exit_syscall(int err_code) {
    (void)err_code;

    kset_color(BLACK, WHITE);
    kprintln("exit syscall");
    return 0;
}

static void* mmap_syscall(void* addr, uint64_t len, ProtFlags prot, int flags, int fd, int64_t offset) {
    (void)flags;        // TODO: more flags default now: MAP_PRIVATE, MAP_ANOM
    (void)fd;           // needed for files
    (void)offset;       // needed for files

    uint64_t pml4_table = get_addr_space();

    PageFlags pageFlags = User | (prot & 3);
    if ((prot & PROT_EXEC) == 0) { pageFlags |= ExecDisable; }

    uint32_t pagesCount = len / PAGE_SIZE + 1;

    void* phys_addr_start = pmm_alloc(pagesCount);
    for (uint32_t i = 0; i < pagesCount; i++) {
        hFrame frame = get_hFrame((uint64_t)phys_addr_start + i*PAGE_SIZE);
        hPage page = get_hPage((uint64_t)addr + i*PAGE_SIZE);

        map_user_frame((uint64_t*) pml4_table, page, frame, pageFlags);
    }

    return (void*)((uint64_t)addr & ~PAGE_SIZE);
}

static uint64_t fork_syscall(void) {
    kset_color(BLACK, WHITE);
    kprintln("fork syscall");
    return 0;
}

static uint64_t execve_syscall(const char* path, const char** argv, const char* envp) {
    (void)path;
    (void)argv;
    (void)envp;

    kset_color(BLACK, WHITE);
    kprintln("execve syscall");
    return 0;
}

static uint64_t getpid_syscall(void) {
    kset_color(BLACK, WHITE);
    kprintln("getpid syscall");
    return 0;
}

static uint64_t kill_syscall(int pid, int sig) {
    (void)pid;
    (void)sig;

    kset_color(BLACK, WHITE);
    kprintln("kill syscall");
    return 0;
}

static uint64_t getcwd_syscall(char* buffer, uint64_t size) {
    (void)buffer;
    (void)size;

    kset_color(BLACK, WHITE);
    kprintln("getcwd syscall");
    return 0;
}

static uint64_t mkdir_syscall(const char* path, uint32_t mode) {
    (void)path;
    (void)mode;

    kset_color(BLACK, WHITE);
    kprintln("mkdir syscall");
    return 0;
}

static uint64_t rmdir_syscall(const char* path) {
    (void)path;

    kset_color(BLACK, WHITE);
    kprintln("rmdir syscall");
    return 0;
}

static uint64_t rename_syscall(const char* oldname, const char* newname) {
    (void)oldname;
    (void)newname;

    kset_color(BLACK, WHITE);
    kprintln("rename syscall");
    return 0;
}

static uint64_t pause_syscall(void) {
    kset_color(BLACK, WHITE);
    kprintln("pause syscall");
    return 0;
}

static uint64_t nano_sleep_syscall(timespec_t* req, timespec_t* rem) {
    (void)req;
    (void)rem;

    kset_color(BLACK, WHITE);
    kprintln("nano sleep syscall");
    return 0;
}

static uint64_t print_int_syscall(int64_t i) {
    kprintln("%d", i);
    return 0;
}


uint64_t syscall_table[SYSCALLS_COUNT] = {
    [SYS_WRITE]         = (uint64_t) &write_syscall,
    [SYS_SCHED_YIELD]   = (uint64_t) &sched_yield_syscall,
    [SYS_OPEN]          = (uint64_t) &open_syscall,
    [SYS_READ]          = (uint64_t) &read_syscall,
    [SYS_CLOSE]         = (uint64_t) &close_syscall,
    [SYS_EXIT]          = (uint64_t) &exit_syscall,
    [SYS_MMAP]          = (uint64_t) &mmap_syscall,
    [SYS_FORK]          = (uint64_t) &fork_syscall,
    [SYS_EXECVE]        = (uint64_t) &execve_syscall,
    [SYS_GETPID]        = (uint64_t) &getpid_syscall,
    [SYS_KILL]          = (uint64_t) &kill_syscall,
    [SYS_GETCWD]        = (uint64_t) &getcwd_syscall,
    [SYS_MKDIR]         = (uint64_t) &mkdir_syscall,
    [SYS_RMDIR]         = (uint64_t) &rmdir_syscall,
    [SYS_RENAME]        = (uint64_t) &rename_syscall,
    [SYS_PAUSE]         = (uint64_t) &pause_syscall,
    [SYS_NANO_SLEEP]    = (uint64_t) &nano_sleep_syscall,

    [SYS_PRINT_INT]     = (uint64_t) &print_int_syscall,
};
