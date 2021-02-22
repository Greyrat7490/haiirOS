#include "hBasicIO.h"

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

static VGA_Buffer* vga_buffer = ( VGA_Buffer* )0xb8000;
static Console console = {
    0,
    0,
    BLACK << 4 | WHITE,
    80,
    25,
};


void clearScreen() 
{
    console.x = 0;
    console.y = 0;

    uint16_t field = console.color << 8;

    for ( uint8_t x = 0; x < console.width; x++ )
        for ( uint8_t y = 0; y < console.height; y++ ) 
            vga_buffer->buffer[y][x] = field;

    console.x = 0;
    console.y = 0;
}

void printByte( uint8_t byte ) 
{   
    if ( byte == '\n' ) 
    {
        console.x = 0;
        console.y++;
    }
    else 
    {
        vga_buffer->buffer[console.y][console.x] = console.color << 8 | byte;

        console.x++;
    }

    if ( console.x >= console.width )
    {
        console.y++;
        console.x = 0;
    }

    if ( console.y >= console.height )
    {
        console.x = 0;
        console.y = 0;
    }
}

void print( const char* str ) {
    while ( *str != '\0' )
    {
        printByte( *str );
        str++;
    }
}

void println( const char* str ) {
    while ( *str != '\0' )
    {
        printByte( *str );
        str++;
    }
    printByte( '\n' );
}

void setColor( uint8_t bg, uint8_t fg ) {
    console.color = bg << 4 | fg;
}