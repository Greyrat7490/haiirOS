#include "isr.h"

__attribute__(( interrupt ))
void interrupt_handler ( struct interrupt_frame* frame ) {
    println( "interrupt!" );
    println( "  instruction pointer: %x", frame->rip );
    println( "  code seg: %x", frame->cs );
    println( "  rflags: %d", frame->rflags );
    println( "  stack pointer: %x", frame->rsp );
    println( "  stack seg: %x", frame->ss );
}

__attribute__(( interrupt ))
void interrupt_handler_err_code ( struct interrupt_frame* frame, uint64_t err_code ) {
    println( "interrupt!" );
    println( "  instruction pointer: %x", frame->rip );
    println( "  code seg: %x", frame->cs );
    println( "  rflags: %d", frame->rflags );
    println( "  stack pointer: %x", frame->rsp );
    println( "  stack seg: %x", frame->ss );
    println( "  err_code: %x", err_code );
}