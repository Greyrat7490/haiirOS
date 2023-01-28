#ifndef H_ISR
#define H_ISR

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


__attribute__((interrupt))
void interrupt_handler(struct interrupt_frame* frame);

__attribute__((interrupt))
void interrupt_handler_err_code(struct interrupt_frame* frame, uint64_t err_code);


static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}
static inline void outw(uint16_t port, uint16_t value) {
    __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}
static inline void outd(uint16_t port, uint32_t value) {
    __asm__ volatile ("out %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t res;
    __asm__ volatile ("inb %1, %0" : "=a"(res) : "Nd"(port));
    return res;
}
static inline uint16_t inw(uint16_t port) {
    uint16_t res;
    __asm__ volatile ("inw %1, %0" : "=a"(res) : "Nd"(port));
    return res;
}
static inline uint32_t ind(uint16_t port) {
    uint32_t res;
    __asm__ volatile ("in %1, %0" : "=a"(res) : "Nd"(port));
    return res;
}

static inline void enable_interrupts(void) {
    __asm__ volatile ("sti");
}

static inline void disable_interrupts(void) {
    __asm__ volatile ("cli");
}

static inline void io_wait(void) {  // wait ~1-4µs
    outb(0x80, 0); // port 0x80 is unused
}

#endif // H_ISR
