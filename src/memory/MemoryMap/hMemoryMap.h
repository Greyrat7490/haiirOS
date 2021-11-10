#ifndef H_MEMORY_MAP
#define H_MEMORY_MAP

#include "types.h"
#include "memory/multiboot2.h"

typedef struct {
    uint32_t kernel_addr;
    struct multiboot_tag_elf_sections* elf_sections;
    struct multiboot_tag_mmap* mapTag;
    multiboot_memory_map_t* first_entry;
} hMemoryMap;


hMemoryMap init_memory_map(uint64_t boot_info_addr);
void print_memory_map(hMemoryMap* mmap);
multiboot_memory_map_t* get_next_mmap_entry(hMemoryMap* mmap, multiboot_memory_map_t* entry);

#endif
