#include "hMemoryMap.h"

#include "io/io.h"
#include "types.h"

hMemoryMap init_memory_map(uint64_t boot_info_addr) {
    uint32_t kernel_addr = 0;
    struct multiboot_tag_mmap* mapTag = 0;
    struct multiboot_tag_elf_sections* elf_sections = 0;
    multiboot_memory_map_t* first_entry = 0;

    struct multiboot_tag* tag = (struct multiboot_tag*) (boot_info_addr + 8);

    while (tag->type != MULTIBOOT_TAG_TYPE_END) {
        switch (tag->type)
        {
        case MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR:
            kernel_addr = ((struct multiboot_tag_load_base_addr*) tag)->load_base_addr;
            break;
        case MULTIBOOT_TAG_TYPE_MMAP:
            {
                mapTag = (struct multiboot_tag_mmap*) tag;
                first_entry = mapTag->entries;
            }
            break;
        case MULTIBOOT_TAG_TYPE_ELF_SECTIONS:
            elf_sections = (struct multiboot_tag_elf_sections*) tag;
            break;
        case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
            println("Bootloader name: %s", ((struct multiboot_tag_string*) tag)->string);
            break;
        case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
            println( "mem_lower: %x, mem_upper: %x",
                ((struct multiboot_tag_basic_meminfo*) tag)->mem_lower,
                ((struct multiboot_tag_basic_meminfo*) tag)->mem_upper
            );
            break;
        default:
            //println( "type %d so far not necessary", tag->type );
            break;
        }

        tag = (struct multiboot_tag*) ((uint8_t*) tag + ((tag->size + 7) & ~7)); // 8bit aligned
    }


    hMemoryMap res = { kernel_addr, elf_sections, mapTag, first_entry };
    return res;
}

void print_memory_map(hMemoryMap* mmap) {
    println("memory_map addr: %x", mmap->mapTag);
    println("kernel addr: %x", mmap->kernel_addr);
    println("elf_sections addr: %x", mmap->elf_sections);

    uint64_t size = 0;
    multiboot_memory_map_t* entry = mmap->first_entry;
    do {
        printf("%x - %x", entry->addr, entry->addr + entry->len);

        if (entry->type == MULTIBOOT_MEMORY_RESERVED)
            println(" (reserved)");
        else if(entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
            size += entry->len;
            println(" (available)");
        } else
            println(" (type %d)", entry->type);

    } while ((entry = get_next_mmap_entry(mmap, entry)) != 0);

    println("Memory size available: %x", size);
}

// 0 means there is no next entry
multiboot_memory_map_t* get_next_mmap_entry(hMemoryMap* mmap, multiboot_memory_map_t* entry) {
    multiboot_memory_map_t* next = (multiboot_memory_map_t*) ((uint64_t) entry + mmap->mapTag->entry_size);

    if ((uint8_t*) next < (uint8_t*) mmap->mapTag + mmap->mapTag->size)
        return next;

    return (multiboot_memory_map_t*)0x0;
}
