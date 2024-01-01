#ifndef H_IDT
#define H_IDT

#include "types.h"

#define CODE_SEG 0x08

#define IST_NONE 0
#define IST_NO_MASKABLE_INT 1
#define IST_DOUBLE_FAULT 2

#define INTERRUPT_GATE  0xe
#define TRAP_GATE 0xf
#define TASK_GATE 0x5

#define DPL_KERNEL 0 // Descriptor Privilege Level (DPL)
#define GATE_PRESENT 1 << 7

struct interrupt_frame {
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};

void init_interrupts(void);
void init_gate(uint8_t idt_index, uint64_t base, uint16_t selector, uint8_t type, uint8_t ist);

#endif // H_IDT
