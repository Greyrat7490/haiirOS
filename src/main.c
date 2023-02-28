#include "types.h"
#include "io/io.h"
#include "proc/task.h"
#include "memory/memory.h"
#include "interrupt/idt.h"
#include "interrupt/bios/bios.h"
#include "pci/pci.h"
#include "driver/ahci/ahci.h"

#include "example_tasks/err_task1.h"
#include "example_tasks/simple.h"

void kernel_main(bloader_boot_info_t* boot_info) {
    kclear_screen();
    kset_color(BLACK, PINK);
    kprintln("%s to %s!", "Welcome", "haiirOS");

    // interrupts, exceptions ---------------------------
    init_bios_services(boot_info);
    select_keyboard_layout(QWERTZ_LAYOUT);
    init_idt();
    // --------------------------------------------------

    // memory --------------------------------------------
    init_memory(boot_info);
    // --------------------------------------------------

    // tests --------------------------------------------
    __asm__ ("int $0x3"); // breakpoint interrupt
    test_mapping();
    kclear_screen();
    // --------------------------------------------------

    // PCI and drivers ----------------------------------
    init_pci();
    init_ahci();
    while(1) __asm__("hlt");
    // --------------------------------------------------

    // start scheduler and go usermode ------------------
    init_syscalls();

    // add_task("task1 causes err", (uint64_t) &err_task);
    add_task("task1", (uint64_t) &task1);
    add_task("task2", (uint64_t) &task2);

    start_scheduler();
    // --------------------------------------------------

    // should never get reached
    while(1) __asm__("hlt");
}
