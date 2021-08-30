#include "io.h"
#include <stdarg.h>

typedef struct 
{
    uint16_t buffer[25][80];
} VGA_Buffer;

typedef struct
{
    uint8_t x;
    uint8_t y;

    uint8_t color;  

    uint8_t width;
    uint8_t height;
} Console;

static volatile VGA_Buffer* vga_buffer = ( VGA_Buffer* )0xb8000;
static Console console = {
    0,
    0,
    BLACK << 4 | WHITE,
    80,
    25,
};


void clear_screen() {
    uint16_t field = console.color << 8;

    for ( uint8_t x = 0; x < console.width; x++ )
        for ( uint8_t y = 0; y < console.height; y++ ) 
            vga_buffer->buffer[y][x] = field;

    console.x = 0;
    console.y = 0;
}

void printByte( uint8_t byte ) 
{                //LF            //CR
    if ( byte == '\n' || byte == '\r' ) {
        console.x = 0;
        console.y++;
    } else if ( byte != '\0' ) {
        vga_buffer->buffer[console.y][console.x] = console.color << 8 | byte;
        console.x++;
    }

    if ( console.x >= console.width ) {
        console.y++;
        console.x = 0;
    }

    if ( console.y >= console.height ) {
        console.x = 0;
        console.y = 0;
    }
}

void printStr( const char* str ) {
    while ( *str != '\0' ) {
        printByte( *str );
        str++;
    }
}

void printInt( int i ) {
    uint8_t buffer[10]; // int max = 2,147,483,647 (10 digits)

    if( i < 0 ) {
        printByte( '-' );
        i = -i;
    }

    int j = 10;
    do {
        buffer[--j] = ( i % 10 ) + 48;
        i /= 10;
    } while( i != 0 );

    for ( uint8_t k = j; k < 10; k++ ) {
        printByte( buffer[k] );
    }
}

void printHex( uint64_t i ) {
    char* repr = "0123456789abcdef";
    uint8_t buffer[16];
    // uint64_t max = 18,446,744,073,709,551,615 = FFFFFFFFFFFFFFFF( 16 chars )

    int j = 16;
    do {
        buffer[--j] = repr[i % 16];
        i /= 16;
    } while( i != 0 );

    printStr( "0x" );
    for ( uint8_t k = j; k < 16; k++ )
        printByte( buffer[k] );
}

void printf( const char* fmt, ... ) {
    va_list args;
    va_start( args, fmt );

    while ( *fmt != '\0' ) {
        if ( *fmt == '%' ) {
            fmt++;

            switch ( *fmt )
            {
            case 's':
                printStr( va_arg( args, char* ) );
                break;
            case 'd':
                printInt( va_arg( args, int ) );
                break;
            case 'x':
                printHex( va_arg( args, uint64_t ) );
                break;
            case 'f':
                printStr( "no float suppport yet" );
                break;
            case 'c':
                printByte( ( char )va_arg( args, int ) );
                break;
            case 'b':
                {
                    if( va_arg( args, int ) )
                        printStr( "true" );
                    else
                        printStr( "false" );
                    break;
                }
            default:
                break;
            }

            fmt++;
        } else {
            printByte( *fmt );
            fmt++;
        }
    }

    va_end( args );
}

void println( const char* fmt, ... ) {
    printf( fmt );
    printByte( '\n' );
}

void set_color( uint8_t bg, uint8_t fg ) {
    console.color = bg << 4 | fg;
}
