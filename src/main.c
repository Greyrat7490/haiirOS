#include "types.h"
#include "io/hBasicIO.h"
#include "memory/memory.h"


void kernel_main( uint64_t boot_info_addr ) {
    clearScreen();
    setColor( BLACK, PINK );

    println( "%s to %s!", "Welcome", "haiirOS" );

    println( "boot_info_addr: %x", boot_info_addr );

    hMemoryMap mmap = init_memory_map( boot_info_addr );

    println( "" );
    print_memory_map( &mmap );

    init_paging();

    clearScreen();

    uint32_t* testAddr = 0x402a12;
    uint16_t* testAddr2 = 0x1000;
    uint16_t* testAddr3 = 0x1;

    println( "testAddr1 virt %x -> phys %x", testAddr, to_phys( testAddr ) );
    println( "testAddr2 virt %x -> phys %x", testAddr2, to_phys( testAddr2 ) );
    println( "testAddr3 virt %x -> phys %x", testAddr3, to_phys( testAddr3 ) );
    
    println( "-------------" );
    println( "testAddr1 val: %d ", *testAddr );
    println( "testAddr2 val: %d ", *testAddr2 );
    println( "testAddr3 val: %d ", *testAddr3 );

    *testAddr = 13;
    *testAddr2 = ( WHITE << 4 | BLACK ) << 8 | 'A';
    *testAddr3 = 25234;

    println( "-------------" );

    println( "testAddr1 val: %d", *testAddr );
    println( "testAddr3 val: %d", *testAddr3 );

    for(;;){}
}
