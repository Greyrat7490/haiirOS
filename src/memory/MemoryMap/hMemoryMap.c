#include "hMemoryMap.h"

hMemoryMap init_memory_map( uint64_t boot_info_addr ) {
    uint32_t kernel_addr;
    struct multiboot_tag_mmap* mapTag;
    struct multiboot_tag_elf_sections* elf_sections;


    struct multiboot_tag* tag = ( struct multiboot_tag* )( boot_info_addr + 8 );

    while ( tag->type != MULTIBOOT_TAG_TYPE_END ) {
        switch ( tag->type )
        {
        case MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR:
            kernel_addr = ( ( struct multiboot_tag_load_base_addr* )tag )->load_base_addr;
            break;
        case MULTIBOOT_TAG_TYPE_MMAP:
            {
                mapTag = ( struct multiboot_tag_mmap* ) tag;
                multiboot_memory_map_t* mmap = mapTag->entries;

                uint32_t entry_size = mapTag->entry_size;
           
                while ( ( uint8_t* )mmap < ( uint8_t* )tag + tag->size ) {
                    printf( "addr_start: %x", mmap->addr );
                    println( " addr_end: %x", mmap->addr + mmap->len );

                    mmap = ( struct multiboot_tag_mmap* )( ( uint64_t )mmap + entry_size );
                }
            }
            break;
        case MULTIBOOT_TAG_TYPE_ELF_SECTIONS:
            elf_sections = ( struct multiboot_tag_elf_sections* ) tag;
            break;
        case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
            println( "Bootloader name: %s", ( ( struct multiboot_tag_string* ) tag )->string );
            break;
        case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
            println( "mem_lower: %x, mem_upper: %x",
                ( ( struct multiboot_tag_basic_meminfo* ) tag )->mem_lower,
                ( ( struct multiboot_tag_basic_meminfo* ) tag )->mem_upper
            );
            break;
        default:
            //println( "type %d so far not necessary", tag->type );
            break;
        }

        tag = ( struct multiboot_tag* ) ( ( uint8_t* ) tag 
            + ( ( tag->size + 7 ) & ~7 ) ); // 8bit aligned
    }


    hMemoryMap res = { kernel_addr, elf_sections, mapTag };
    return res;
}

void print_memory_map( hMemoryMap* mmap ) {
    println( "memory_map addr: %x", mmap->mapTag );
    println( "kernel addr: %x", mmap->kernel_addr );
    println( "elf_sections addr: %x", mmap->elf_sections );
}
