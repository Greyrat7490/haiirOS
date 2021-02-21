#include "types.h"
#include "io/hBasicIO.h"

void kernel_main( void ) {
    struct VGA_Buffer* vga_buffer = ( struct VGA_Buffer* )0xb8000;

    for ( uint8_t x = 0; x < 80; x++ ) {
        for ( uint8_t y = 0; y < 25; y++ ) {
            vga_buffer->buffer[x][y] = DARK_GREY << 12;
        }
    }

    for(;;){}
}