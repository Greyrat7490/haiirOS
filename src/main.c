#include "memory/memory.h"
#include "interrupt/idt.h"
#include "proc/task.h"
#include "io/io.h"
#include "types.h"

extern void test_user_function() {
    while(1){};
}

extern void jump_usermode(uint64_t usr_stack_top, uint64_t usr_func_addr);


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

    // TODO: create paging tabels for user/task and load into cr3
    // add_task();
    // start_scheduler();

    // map stack
    uint64_t user_stack_top = 0x1000000f000;
    for (int i = 0; i < 7; i++) {
        hFrame frame = alloc_frame();
        hPage page = get_hPage(user_stack_top - i * 0x1000);
        map_to(page, frame, Present | Writeable | User);
    }

    // map test_user_function
    uint64_t user_func_addr = 0x10000000000 + ((uint64_t) &test_user_function & 0xfff);
    hFrame frame = get_hFrame((uint64_t) &test_user_function);
    hPage page = get_hPage(user_func_addr);
    map_to(page, frame, Present | Writeable | User);

    println("virtual user_func_addr: %x", user_func_addr);


    println("going into user mode...");
    jump_usermode(user_stack_top, user_func_addr);
    // --------------------------------------------------

    // should never get reached
    while(1) __asm__("hlt");
}
