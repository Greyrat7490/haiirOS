#include "idt.h"
#include "io/hBasicIO.h"

struct IDT_descr {
    uint16_t offset1;
    uint16_t selector;
    uint8_t ist;
    uint8_t type;
    uint16_t offset2;
    uint32_t offset3;
    uint32_t zero;
};

typedef struct
{
    struct IDT_descr entries[256];
} IDT;

struct interrupt_frame {
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};


static IDT* idt;

#define CODE_SEG 0x08
#define TSS_SEG 0x10

#define IST_NONE 0
#define IST_NO_MASKABLE_INT 1
#define IST_DOUBLE_FAULT 2

#define INTERRUPT_GATE  0xe
#define TRAP_GATE 0xf
#define TASK_GATE 0x5

#define DPL_KERNEL 0 // Descriptor Privilege Level( DPL )
#define GATE_PRESENT 1 << 7


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


void init_gate( uint8_t idt_index, uint64_t base, uint16_t selector, uint8_t type, uint8_t ist ) {
    idt->entries[idt_index].zero = 0;
    
    idt->entries[idt_index].offset1 = ( base & 0xffff );
    idt->entries[idt_index].offset2 = ( ( base >> 16 ) & 0xffff );
    idt->entries[idt_index].offset3 = ( ( base >> 32 ) & 0xffffffff );
    
    idt->entries[idt_index].selector = selector;
    idt->entries[idt_index].type = type;
    idt->entries[idt_index].ist = ist;
}

void init_idt( uint64_t idt_addr ) {
    idt = ( IDT* )idt_addr;

    // first 32 are for exceptions ---------------------
    for ( uint64_t i = 0; i < 31; i++ )
        init_gate( i, ( uint64_t )interrupt_handler, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE );
    
    init_gate( 2, ( uint64_t )interrupt_handler, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NO_MASKABLE_INT );
    // -------------------------------------------------

    // for exceptions with err code --------------------
    init_gate( 8, ( uint64_t )interrupt_handler_err_code, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_DOUBLE_FAULT );
    
    for ( uint64_t i = 9; i < 13; i++ )
        init_gate( i, ( uint64_t )interrupt_handler_err_code, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE );
    
    init_gate( 15, ( uint64_t )interrupt_handler_err_code, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE );
    init_gate( 19, ( uint64_t )interrupt_handler_err_code, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE );
    // -------------------------------------------------
    
    // rest --------------------------------------------
    for ( uint64_t i = 32; i < 256; i++ )
        init_gate( i, ( uint64_t )interrupt_handler, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE );
    // -------------------------------------------------


    struct {
        uint16_t length;
        uint64_t addr;
    } __attribute__(( packed )) IDTR = { sizeof( IDT ) - 1, idt_addr };

    __asm__ volatile ( "lidt %0" : : "m"( IDTR ) );
}