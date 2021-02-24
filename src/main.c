#include "types.h"
#include "io/hBasicIO.h"
#include "memory/multiboot2.h"
#include "memory/hMemoryMap.h"

uint32_t physical_offset = 0;

void kernel_main( uint64_t boot_info_addr ) {
    clearScreen();
    setColor( BLACK, PINK );

    println( "%s to %s!", "Welcome", "haiirOS" );

    println( "boot_info_addr: %x", boot_info_addr );  
    println( "physical_offset: %d", physical_offset );

    hMemoryMap mmap = init_memory_map( boot_info_addr );

    println( "" );
    print_memory_map( &mmap );

    for(;;){}
}
