#ifndef H_IDT
#define H_IDT

#include "types.h"

void init_idt(void);
void init_gate(uint8_t idt_index, uint64_t base, uint16_t selector, uint8_t type, uint8_t ist);
void remap_pic(void);

#endif // H_IDT
