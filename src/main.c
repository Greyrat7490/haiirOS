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
    // print_memory_map();

    kprintln("");

    print_frame_map();
    kprintln("");

    void* test = pmm_alloc(3);

    print_frame_map();
    kprintln("");

    pmm_free(to_frame((uint64_t)test), 3);

    print_frame_map();

    while(1) __asm__("hlt");
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

    // test vbe -----------------------------------------
    set_vbe_mode(boot_info->vbe_mode);

    uint32_t* volatile framebuffer = (uint32_t*)(uint64_t)boot_info->vbe_mode->framebuffer;
    uint32_t pitch = boot_info->vbe_mode->pitch;
    uint32_t color = 0x41336e;
    uint32_t color2 = 0x5597ce;

    while (1) {
        for (uint32_t y = 0; y < boot_info->vbe_mode->height; y++) {
            for (uint32_t x = 0; x < boot_info->vbe_mode->width; x++) {
                framebuffer[x + y*pitch/4] = color;
            }
        }

        for(uint32_t i = 0; i < 16; i++) __asm__ volatile ("hlt");

        for (uint32_t y = 0; y < boot_info->vbe_mode->height; y++) {
            for (uint32_t x = 0; x < boot_info->vbe_mode->width; x++) {
                framebuffer[x + y*pitch/4] = color2;
            }
        }

        for(uint32_t i = 0; i < 16; i++) __asm__ volatile ("hlt");
    }
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
