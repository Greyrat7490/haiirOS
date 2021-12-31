#include "isr.h"
#include "io/io.h"

__attribute__((interrupt))
void interrupt_handler (struct interrupt_frame* frame) {
    kprintln("interrupt!");
    kprintln("  instruction pointer: %x", frame->rip);
    kprintln("  code seg: %x", frame->cs);
    kprintln("  rflags: %d", frame->rflags);
    kprintln("  stack pointer: %x", frame->rsp);
    kprintln("  stack seg: %x", frame->ss);
}

__attribute__((interrupt))
void interrupt_handler_err_code (struct interrupt_frame* frame, uint64_t err_code) {
    kprintln("interrupt!");
    kprintln("  instruction pointer: %x", frame->rip);
    kprintln("  code seg: %x", frame->cs);
    kprintln("  rflags: %d", frame->rflags);
    kprintln("  stack pointer: %x", frame->rsp);
    kprintln("  stack seg: %x", frame->ss);
    kprintln("  err_code: %x", err_code);
}
