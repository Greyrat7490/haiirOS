#ifndef H_IO
#define H_IO

#include "types.h"
#include "input.h"

void clear_screen();
void printf(const char* fmt, ...);
void println(const char* fmt, ...);
void set_color(uint8_t background, uint8_t foreground);

#endif // H_IO
