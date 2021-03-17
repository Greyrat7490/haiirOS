#include "input.h"
#include "keyboard/qwertz_keyboard.h"
#include "io/io.h"

static struct {
    enum keycode keycodes[256];
    char oem_table[13];
    unsigned char numpad_table[6];
} keyboard;

enum keystate keys[128] = {0};

void select_keyboard_layout( enum keyboard_layouts layout ) {
    if ( layout == QWERTZ_LAYOUT ) {

        for( uint8_t i = 0; i < 255; i++ )
            keyboard.keycodes[i] = qwertz_keycodes[i];
    
        for( uint8_t i = 0; i < 13; i++ )
            keyboard.oem_table[i] = qwertz_oem_table[i];

        for( uint8_t i = 0; i < 6; i++ )
            keyboard.numpad_table[i] = qwertz_numpad_table[i];

        println( "Keyboard layout QWERTZ" );
    }
    else
        println( "Keyboard layout is not supported( so far )" );
}

char scancode_to_ascii( uint8_t scancode ) {
    enum keycode key = keyboard.keycodes[scancode];

    if( ( key <= 90 && key >= 27 ) || ( key <= 13 && key >= 8 ) )
        return ( char )key;
    else if( key > 90 && key < 104 )
        return ( char )keyboard.oem_table[( int )key - 91];
    else if( key >= 141 && key <= 150 )     // numpad digits
        return ( char )( ( int )key - 93 );
    else if( key > 150 && key <= 156 )      // other numpad keys
        return ( char )keyboard.numpad_table[( int )key - 151];

    return '\0'; // rest is undefined
}