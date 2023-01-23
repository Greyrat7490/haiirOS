#include "memory/Paging/Page/hPage.h"
#include "types.h"
#include "io/io.h"
#include "proc/task.h"
#include "memory/memory.h"
#include "interrupt/idt.h"
#include "interrupt/bios/bios.h"

#include "example_tasks/err_task1.h"
#include "example_tasks/simple.h"

void kernel_main(bloader_boot_info_t* boot_info) {
    kclear_screen();
    kset_color(BLACK, PINK);

    kprintln("%s to %s!", "Welcome", "haiirOS");

    init_bios_services(boot_info);
    // memory --------------------------------------------
    memory_info_t mmap = init_memory_map(boot_info);

    // print_memory_map(&mmap);

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


    // test vbe -----------------------------------------
    set_vbe_mode(boot_info->vbe_mode);

    uint32_t* volatile framebuffer = (uint32_t*)(uint64_t)boot_info->vbe_mode->framebuffer;
    map_frame(get_hPage((uint64_t)framebuffer), get_hFrame((uint64_t)framebuffer), Present | Writeable);

    for (uint32_t i = 0; i < boot_info->vbe_mode->width * boot_info->vbe_mode->height; i++) {
        *(framebuffer + i) = 0x41336e;
    }

    while(1) __asm__("hlt");
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
