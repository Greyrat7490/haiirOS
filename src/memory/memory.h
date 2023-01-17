#ifndef MEMORY_H_
#define MEMORY_H_

#include "types.h"
#include "boot/boot_info.h"
#include "Paging/hPaging.h"

typedef struct {
    memory_map_t map;
    uint32_t mem_lower; // in KiB
    uint32_t mem_upper; // in KiB
    uint32_t kernel_addr;
    uint32_t kernel_size;
    uint32_t vbe_framebuffer;
    uint32_t vbe_framebuffer_size;
} memory_info_t;

memory_info_t init_memory_map(bloader_boot_info_t* boot_info);
void print_memory_map(memory_info_t* memory_info);

#endif // MEMORY_H_
