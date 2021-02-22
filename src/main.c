#include "types.h"
#include "io/hBasicIO.h"

void kernel_main( void ) {
    clearScreen();
    setColor( BLACK, PINK );

    println( "%s to %s!", "Welcome", "haiirOS" );

    println( "test hex %x", 0x20ffa31 );

    println( "test dec %d", -1412349087 );

    // floats need sse in x86, but sse has to be disabled in kernel mode
    // TODO: more research and maybe a documentation
    println( "test float %f", 14.4f );

    printf( "test1" );
    println( "test2" );

    setColor( WHITE, BLACK );
    printf( "test3" );

    for(;;){}
}