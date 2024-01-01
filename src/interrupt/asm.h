#ifndef INTERRUPT_ASM_H_
#define INTERRUPT_ASM_H_

#include "types.h"

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

static inline uint64_t read_msr(uint32_t id) {
    uint64_t high, low;
    __asm__ volatile("rdmsr" : "=a" (low), "=d" (high) : "c" (id));
	return (high << 32) | low;
}

static inline void write_msr(uint32_t id, uint64_t value) {
    uint32_t low = value;
    uint32_t high = value >> 32;
    __asm__ volatile ("wrmsr" : : "c" (id), "a" (low), "d" (high));
}


#endif // !INTERRUPT_ASM_H_
