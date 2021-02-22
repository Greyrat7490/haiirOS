#ifndef H_BASIC_IO_H
#define H_BASIC_IO_H

#include "../types.h"

enum ASCII_color {
    BLACK,
    BLUE,
    GREEN,
    CYAN,
    RED,
    PURPLE,
    BROWN,
    GREY,
    DARK_GREY,
    LIGHT_BLUE,
    LIGHT_GREEN,
    LIGHT_CYAN,
    LIGHT_RED,
    PINK,
    YELLOW,
    WHITE,
};

void clearScreen();
void print( const char* );
void println( const char* );
void setColor( uint8_t bg, uint8_t fg );

#endif