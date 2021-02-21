#ifndef H_BASIC_IO_H
#define H_BASIC_IO_H

#include "../types.h"

enum ASCII_color {
    BLACK,
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    BROWN,
    GREY,
    DARK_GREY,
    LIGHT_BLUE,
    LIGHT_GREEN,
    LIGHT_CYAN,
    LIGHT_RED,
    LIGHT_MAGENTA,
    YELLOW,
    WHITE,
};

struct VGA_Buffer {
    uint16_t buffer[80][25];
};

#endif