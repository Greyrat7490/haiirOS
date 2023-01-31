#include "memory.h"
#include "io/io.h"
#include "memory/Paging/Frame/hFrame.h"

static memory_info_t memory_info;

static void init_memory_map(bloader_boot_info_t* boot_info) {
    vbe_mode_info_t* mode = boot_info->vbe_mode;

    memory_info = (memory_info_t) {
        .map = boot_info->memory_map,
        .mem_lower = boot_info->lower_memory_KiB,
        .mem_upper = (uint32_t)boot_info->upper_memory_64KiB * 64,
        .kernel_addr = boot_info->kernel_addr,
        .kernel_size = boot_info->kernel_size,
        .vbe_framebuffer = boot_info->vbe_mode->framebuffer,
        .vbe_framebuffer_size = mode->width * mode->height * mode->bpp
    };
}

void init_memory(bloader_boot_info_t* boot_info) {
    init_memory_map(boot_info);
    init_frame_allocator(&memory_info);
}

void print_memory_map(void) {
    memory_map_t map = memory_info.map;

    kprintln("memory_map addr: %x", map.entries);
    kprintln("kernel addr: %x", memory_info.kernel_addr);

    uint64_t size = 0;
    for (uint16_t i = 0; i < map.count; i++) {
        kprintf("%x - %x", map.entries[i].base, map.entries[i].base + map.entries[i].len);

        switch (map.entries[i].type) {
        case MEMORY_RANGE_USABLE:
            kprintln(" (available)");
            size += map.entries[i].len;
            break;
        case MEMORY_RANGE_RESERVED:
            kprintln(" (reserved)");
            break;
        default:
            kprintln(" (type %d)", map.entries[i].type);
        }
    }

    kprintln("Memory size available: %x", size);
}
