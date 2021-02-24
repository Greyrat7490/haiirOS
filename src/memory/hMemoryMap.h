#ifndef H_MEMORY_MAP
#define H_MEMORY_MAP

#include "../types.h"
#include "multiboot2.h"
#include "../io/hBasicIO.h"

typedef struct {
    uint32_t kernel_addr;
    struct multiboot_tag_elf_sections* elf_sections;
    struct multiboot_tag_mmap* mapTag;
} hMemoryMap;


hMemoryMap init_memory_map( uint64_t boot_info_addr );
void print_memory_map( hMemoryMap* mmap );

#endif