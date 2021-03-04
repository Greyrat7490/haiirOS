#include "types.h"
#include "io/hBasicIO.h"
#include "memory/memory.h"
#include "interrupt/idt.h"


void kernel_main( uint64_t boot_info_addr ) {
    clear_screen();
    set_color( BLACK, PINK );

    println( "%s to %s!", "Welcome", "haiirOS" );

    hMemoryMap mmap = init_memory_map( boot_info_addr );

    init_paging();
    
    init_idt( 0x124000 ); // IDT from 0x124000 - 0x125000

    __asm__ volatile ( "int $0x3" );

    //println( "%d", 13 / 0 );

    for(;;){}
}