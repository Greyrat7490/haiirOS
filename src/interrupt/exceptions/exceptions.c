#include "exceptions.h"

#include "types.h"
#include "io/io.h"

#include "interrupt/ISR/isr.h"
#include "interrupt/idt.h"


__attribute__((interrupt))
static void exc0 (struct interrupt_frame* frame) {
    kprintln("Exception0: divide by zero");
    kprintln("  instruction pointer: %x", frame->rip);
    kprintln("  code seg: %x", frame->cs);
    kprintln("  rflags: %d", frame->rflags);
    kprintln("  stack pointer: %x", frame->rsp);
    kprintln("  stack seg: %x", frame->ss);
    __asm__("hlt");
}

__attribute__((interrupt))
static void exc1 (struct interrupt_frame* frame) {
    kprintln("Exception1: debug");
    kprintln("  instruction pointer: %x", frame->rip);
    kprintln("  code seg: %x", frame->cs);
    kprintln("  rflags: %d", frame->rflags);
    kprintln("  stack pointer: %x", frame->rsp);
    kprintln("  stack seg: %x", frame->ss);
}

__attribute__((interrupt))
static void exc2 (struct interrupt_frame* frame) {
    kprintln("Exception2: non maskable interrupt");
    kprintln("  instruction pointer: %x", frame->rip);
    kprintln("  code seg: %x", frame->cs);
    kprintln("  rflags: %d", frame->rflags);
    kprintln("  stack pointer: %x", frame->rsp);
    kprintln("  stack seg: %x", frame->ss);
    __asm__("hlt");
}

__attribute__((interrupt))
static void exc3 (struct interrupt_frame* frame) {
    kprintln("Exception3: breakpoint");
    kprintln("  instruction pointer: %x", frame->rip);
    kprintln("  code seg: %x", frame->cs);
    kprintln("  rflags: %d", frame->rflags);
    kprintln("  stack pointer: %x", frame->rsp);
    kprintln("  stack seg: %x", frame->ss);
}

__attribute__((interrupt))
static void exc4 (struct interrupt_frame* frame) {
    kprintln("Exception4: overflow");
    kprintln("  instruction pointer: %x", frame->rip);
    kprintln("  code seg: %x", frame->cs);
    kprintln("  rflags: %d", frame->rflags);
    kprintln("  stack pointer: %x", frame->rsp);
    kprintln("  stack seg: %x", frame->ss);
    __asm__("hlt");
}

__attribute__((interrupt))
static void exc5 (struct interrupt_frame* frame) {
    kprintln("Exception5: bound_range_overflow");
    kprintln("  instruction pointer: %x", frame->rip);
    kprintln("  code seg: %x", frame->cs);
    kprintln("  rflags: %d", frame->rflags);
    kprintln("  stack pointer: %x", frame->rsp);
    kprintln("  stack seg: %x", frame->ss);
    __asm__("hlt");
}

__attribute__((interrupt))
static void exc6 (struct interrupt_frame* frame) {
    kprintln("Exception6: invalid_opcode");
    kprintln("  instruction pointer: %x", frame->rip);
    kprintln("  code seg: %x", frame->cs);
    kprintln("  rflags: %d", frame->rflags);
    kprintln("  stack pointer: %x", frame->rsp);
    kprintln("  stack seg: %x", frame->ss);
    __asm__("hlt");
}

__attribute__((interrupt))
static void exc7 (struct interrupt_frame* frame) {
    kprintln("Exception7: device not available");
    kprintln("  instruction pointer: %x", frame->rip);
    kprintln("  code seg: %x", frame->cs);
    kprintln("  rflags: %d", frame->rflags);
    kprintln("  stack pointer: %x", frame->rsp);
    kprintln("  stack seg: %x", frame->ss);
    __asm__("hlt");
}

__attribute__((interrupt))
static void exc8 (struct interrupt_frame* frame, uint64_t err_code) {
    kprintln("Exception8: double fault");
    kprintln("  instruction pointer: %x", frame->rip);
    kprintln("  code seg: %x", frame->cs);
    kprintln("  rflags: %d", frame->rflags);
    kprintln("  stack pointer: %x", frame->rsp);
    kprintln("  stack seg: %x", frame->ss);
    kprintln("  err_code: %x", err_code);
    __asm__("hlt");
}

__attribute__((interrupt))
static void exc9 (struct interrupt_frame* frame) {
    kprintln("Exception9: reserved");
    kprintln("  instruction pointer: %x", frame->rip);
    kprintln("  code seg: %x", frame->cs);
    kprintln("  rflags: %d", frame->rflags);
    kprintln("  stack pointer: %x", frame->rsp);
    kprintln("  stack seg: %x", frame->ss);
}

__attribute__((interrupt))
static void exc10 (struct interrupt_frame* frame, uint64_t err_code) {
    kprintln("Exception10: invalid_tss");
    kprintln("  instruction pointer: %x", frame->rip);
    kprintln("  code seg: %x", frame->cs);
    kprintln("  rflags: %d", frame->rflags);
    kprintln("  stack pointer: %x", frame->rsp);
    kprintln("  stack seg: %x", frame->ss);
    kprintln("  err_code: %x", err_code);
    __asm__("hlt");
}

__attribute__((interrupt))
static void exc11 (struct interrupt_frame* frame, uint64_t err_code) {
    kprintln("Exception11: segment_not_present");
    kprintln("  instruction pointer: %x", frame->rip);
    kprintln("  code seg: %x", frame->cs);
    kprintln("  rflags: %d", frame->rflags);
    kprintln("  stack pointer: %x", frame->rsp);
    kprintln("  stack seg: %x", frame->ss);
    kprintln("  err_code: %x", err_code);
    __asm__("hlt");
}

__attribute__((interrupt))
static void exc12 (struct interrupt_frame* frame, uint64_t err_code) {
    kprintln("Exception12: stack_segment_fault");
    kprintln("  instruction pointer: %x", frame->rip);
    kprintln("  code seg: %x", frame->cs);
    kprintln("  rflags: %d", frame->rflags);
    kprintln("  stack pointer: %x", frame->rsp);
    kprintln("  stack seg: %x", frame->ss);
    kprintln("  err_code: %x", err_code);
    __asm__("hlt");
}
__attribute__(( interrupt ))

static void exc13( struct interrupt_frame* frame, uint64_t err_code ) {
    kprintln( "Exception13: general_protection_fault");
    kprintln( "  instructin pointer: %x", frame->rip );
    kprintln("  code seg: %x", frame->cs);
    kprintln("  rflags: %d", frame->rflags);
    kprintln("  stack pointer: %x", frame->rsp);
    kprintln("  stack seg: %x", frame->ss);
    kprintln("  err_code: %x", err_code);
    __asm__("hlt");
}

__attribute__((interrupt))
static void exc14 (struct interrupt_frame* frame, uint64_t err_code ) {
    kprintln("Exception14: page_fault");
    kprintln("  instruction pointer: %x", frame->rip);
    kprintln("  code seg: %x", frame->cs);
    kprintln("  rflags: %d", frame->rflags);
    kprintln("  stack pointer: %x", frame->rsp);
    kprintln("  stack seg: %x", frame->ss);
    kprintln("  err_code: %x", err_code);

    uint64_t addr = 0;
    __asm__ (
        "mov %%cr2, %%rax\n" // cr2 -> rax
        "mov %%rax, %0\n"   // rax -> addr
        : "=m" ( addr )
        : // no input
        : "%rax"
    );

    kprintln( "  at virt_addr: %x", addr );

    __asm__("hlt");
}

__attribute__((interrupt))
static void exc15 (struct interrupt_frame* frame) {
    kprintln("Exception15: reserved");
    kprintln("  instruction pointer: %x", frame->rip);
    kprintln("  code seg: %x", frame->cs);
    kprintln("  rflags: %d", frame->rflags);
    kprintln("  stack pointer: %x", frame->rsp);
    kprintln("  stack seg: %x", frame->ss);
}

__attribute__((interrupt))
static void exc16 (struct interrupt_frame* frame) {
    kprintln("Exception16: x87 floating-point");
    kprintln("  instruction pointer: %x", frame->rip);
    kprintln("  code seg: %x", frame->cs);
    kprintln("  rflags: %d", frame->rflags);
    kprintln("  stack pointer: %x", frame->rsp);
    kprintln("  stack seg: %x", frame->ss);
    __asm__("hlt");
}

__attribute__((interrupt))
static void exc17 (struct interrupt_frame* frame, uint64_t err_code) {
    kprintln("Exception15: alignment_check");
    kprintln("  instruction pointer: %x", frame->rip);
    kprintln("  code seg: %x", frame->cs);
    kprintln("  rflags: %d", frame->rflags);
    kprintln("  stack pointer: %x", frame->rsp);
    kprintln("  stack seg: %x", frame->ss);
    kprintln("  err_code: %x", err_code);
    __asm__("hlt");
}

__attribute__((interrupt))
static void exc18 (struct interrupt_frame* frame) {
    kprintln("Exception18: machine_check");
    kprintln("  instruction pointer: %x", frame->rip);
    kprintln("  code seg: %x", frame->cs);
    kprintln("  rflags: %d", frame->rflags);
    kprintln("  stack pointer: %x", frame->rsp);
    kprintln("  stack seg: %x", frame->ss);
    __asm__("hlt");
}

__attribute__((interrupt))
static void exc19 (struct interrupt_frame* frame) {
    kprintln("Exception19: simd_floating_point");
    kprintln("  instruction pointer: %x", frame->rip);
    kprintln("  code seg: %x", frame->cs);
    kprintln("  rflags: %d", frame->rflags);
    kprintln("  stack pointer: %x", frame->rsp);
    kprintln("  stack seg: %x", frame->ss);
    __asm__("hlt");
}

// __attribute__((interrupt))
// static void exc20 (struct interrupt_frame* frame) {
//     println("Exception20: virtualization");
//     println("  instruction pointer: %x", frame->rip);
//     println("  code seg: %x", frame->cs);
//     println("  rflags: %d", frame->rflags);
//     println("  stack pointer: %x", frame->rsp);
//     println("  stack seg: %x", frame->ss);
// }
//
// __attribute__((interrupt))
// static void exc21 (struct interrupt_frame* frame) {
//     println("Exception21: reserved");
//     println("  instruction pointer: %x", frame->rip);
//     println("  code seg: %x", frame->cs);
//     println("  rflags: %d", frame->rflags);
//     println("  stack pointer: %x", frame->rsp);
//     println("  stack seg: %x", frame->ss);
// }



void init_exceptions(void) {
    init_gate(0,  (uint64_t) exc0,  CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE);
    init_gate(1,  (uint64_t) exc1,  CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE);
    init_gate(2,  (uint64_t) exc2,  CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NO_MASKABLE_INT);
    init_gate(3,  (uint64_t) exc3,  CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE);
    init_gate(4,  (uint64_t) exc4,  CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE);
    init_gate(5,  (uint64_t) exc5,  CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE);
    init_gate(6,  (uint64_t) exc6,  CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE);
    init_gate(7,  (uint64_t) exc7,  CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE);
    init_gate(8,  (uint64_t) exc8,  CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_DOUBLE_FAULT);
    init_gate(9,  (uint64_t) exc9,  CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE);
    init_gate(10, (uint64_t) exc10, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE);
    init_gate(11, (uint64_t) exc11, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE);
    init_gate(12, (uint64_t) exc12, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE);
    init_gate(13, (uint64_t) exc13, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE);
    init_gate(14, (uint64_t) exc14, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE);
    init_gate(15, (uint64_t) exc15, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE);
    init_gate(16, (uint64_t) exc16, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE);
    init_gate(17, (uint64_t) exc17, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE);
    init_gate(18, (uint64_t) exc18, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE);
    init_gate(19, (uint64_t) exc19, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE);

    for (uint64_t i = 20; i < 31; i++)
        init_gate(i, (uint64_t) interrupt_handler, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE);
}
