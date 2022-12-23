#ifndef H_IO
#define H_IO

#include "types.h"
#include "input.h"

void kclear_screen(void);
void kprintf(const char* fmt, ...);
void kprintln(const char* fmt, ...);
void kset_color(uint8_t background, uint8_t foreground);

#endif // H_IO
