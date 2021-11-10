#include "interrupt/ISR/isr.h"
#include "memory/MemoryMap/hMemoryMap.h"
#include "memory/Paging/Frame/hFrame.h"
#include "memory/Paging/hPaging.h"
#include "proc/task.h"
#include "types.h"
#include "io/io.h"
#include "memory/memory.h"
#include "interrupt/idt.h"

extern void test_user_function() {
    while(1){};
}

extern void jump_usermode();

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
    // --------------------------------------------------


    // start scheduler and go usermode ------------------ 
    init_tss();
 
    // TODO: create paging tabels for user/task and load into cr3
/*     hFrame test_func_frame = get_hFrame((uint64_t)test_user_function);
 *     hPage test_func_page = get_hPage((uint64_t)test_user_function);
 *     map_to(test_func_page, test_func_frame, Present | Writeable | User);
 *
 *     // usr_stack 0x121000 - 0x11b000
 *     extern uint64_t usr_stack;
 *     println("user stack addr top: %x", usr_stack);
 *     println("user stack addr bottom: %x", usr_stack - 4096 * 6);
 *
 *     for (int i = 0; i < 7; i++) {
 *         hFrame frame = get_hFrame(usr_stack - i * 0x1000);
 *         hPage page = get_hPage(usr_stack - i * 0x1000);
 *         map_to(page, frame, Present | Writeable | User);
 *     } */
    // add_task();
    // start_scheduler();
    
    println("going into user mode...");
    jump_usermode();
    // --------------------------------------------------
    
    // should never get reached
    while(1) __asm__("hlt");
}
