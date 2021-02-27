#include "types.h"
#include "io/hBasicIO.h"
#include "memory/memory.h"


void kernel_main( uint64_t boot_info_addr ) {
    clear_screen();
    set_color( BLACK, PINK );

    println( "%s to %s!", "Welcome", "haiirOS" );

    println( "boot_info_addr: %x", boot_info_addr );

    hMemoryMap mmap = init_memory_map( boot_info_addr );

    println( "" );
    print_memory_map( &mmap );

    init_paging();

    clear_screen();

    // would cause a panic, because 0x0 - 0xfff is still unmapped
    // uint16_t* unmappedAddr = 0x10;
    // *unmappedAddr = 13;

    test_mapping();

    for(;;){}
}
