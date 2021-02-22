#include "types.h"
#include "io/hBasicIO.h"

void kernel_main( void ) {
    clearScreen();
    setColor( BLACK, PINK );

    println( "Welcome to haiirOS!" );

    print( "test1" );
    println( "test2" );

    setColor( WHITE, BLACK );
    print( "test3" );

    for(;;){}
}