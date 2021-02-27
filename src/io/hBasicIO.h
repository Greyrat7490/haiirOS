#ifndef H_BASIC_IO_H
#define H_BASIC_IO_H

#include "types.h"

void clear_screen();
void printf( const char* fmt, ... );
void println( const char* fmt, ... );
void set_color( uint8_t background, uint8_t foreground );

#endif