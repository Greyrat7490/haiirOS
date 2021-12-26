#include "types.h"
#include "io/io.h"
#include "proc/task.h"
#include "memory/memory.h"
#include "interrupt/idt.h"
#include "syscall/syscall.h"


void test_syscall() {
    set_color(BLACK, WHITE);
    println("test syscall");
}

void test_user_function() {
    syscall();
    syscall();

    while(1){};
}


void kernel_main(uint64_t boot_info_addr) {
    clear_screen();
    set_color(BLACK, PINK);

    println("%s to %s!", "Welcome", "haiirOS");

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

    clear_screen();
    // --------------------------------------------------

    // start scheduler and go usermode ------------------
    init_tss();
    init_syscalls();

    add_task("first task", (uint64_t) &test_user_function);

    start_scheduler();
    // --------------------------------------------------

    // should never get reached
    while(1) __asm__("hlt");
}
