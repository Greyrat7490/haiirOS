#include "types.h"
#include "io/io.h"
#include "proc/task.h"
#include "memory/memory.h"
#include "interrupt/idt.h"

#include "example_tasks/err_task1.h"
#include "example_tasks/simple.h"

void kernel_main(uint64_t boot_info_addr) {
    kclear_screen();
    kset_color(BLACK, PINK);

    kprintln("%s to %s!", "Welcome", "haiirOS");

    // memory --------------------------------------------
    hMemoryMap mmap = init_memory_map(boot_info_addr);

    print_memory_map(&mmap);

    init_paging();
    // --------------------------------------------------

    // interrupts, exceptions ---------------------------
    select_keyboard_layout(QWERTZ_LAYOUT);

    init_idt();
    // --------------------------------------------------

    // tests --------------------------------------------
    __asm__ ("int $0x3"); // breakpoint interrupt

    test_mapping();

    kclear_screen();
    // --------------------------------------------------

    // start scheduler and go usermode ------------------
    init_tss();
    init_syscalls();

    // add_task("task1 causes err", (uint64_t) &err_task);
    add_task("task1", (uint64_t) &task1);
    add_task("task2", (uint64_t) &task2);

    start_scheduler();
    // --------------------------------------------------

    // should never get reached
    while(1) __asm__("hlt");
}
