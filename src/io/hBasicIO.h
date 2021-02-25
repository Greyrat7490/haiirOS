#ifndef H_BASIC_IO_H
#define H_BASIC_IO_H

#include "types.h"

void clearScreen();
void printf( const char* fmt, ... );
void println( const char* fmt, ... );
void setColor( uint8_t background, uint8_t foreground );

#endif