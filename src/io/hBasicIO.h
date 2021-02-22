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
void printf( const char* fmt, ... );
void println( const char* fmt, ... );
void setColor( uint8_t background, uint8_t foreground );

#endif