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

//#define WRITE_TEST
//#define VBE_TEST

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

    ahci_dev_t* ahci_dev = get_ahci_dev(0, 0);
                                // test reading more sectors than available
    char* buffer = pmm_alloc((ahci_dev->sector_count*AHCI_SECTOR_SIZE+1) / FRAME_SIZE + 1);
    uint64_t res = ahci_read(0, ahci_dev->sector_count*AHCI_SECTOR_SIZE+1, buffer, ahci_dev);
    kprintln("read: '%s'", buffer+3);
    kprintln("read: %x bytes", res);

    /*
     * WARNING DO NOT TEST ON YOUR REAL DEVICE OR YOUR BOOT SECTOR GETS OVERWRITEN
     */
#ifdef WRITE_TEST
    kprintln("WARNING DO NOT TEST ON YOUR REAL DEVICE OR YOUR BOOT SECTOR GETS OVERWRITEN");

    const char* msg = "   test message";
    kprintln("write: '%s'", msg);
    res = ahci_write(0, AHCI_SECTOR_SIZE, (void*)msg, ahci_dev);
    kprintln("wrote: %d bytes", res);

    res = ahci_read(0, AHCI_SECTOR_SIZE, buffer, ahci_dev);
    kprintln("read: '%s'", buffer+3);
    kprintln("read: %d bytes", res);
#endif

    while(1) __asm__("hlt");
    // --------------------------------------------------

    // test vbe -----------------------------------------

#ifdef VBE_TEST
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
#endif
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
