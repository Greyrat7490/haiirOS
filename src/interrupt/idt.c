#include "idt.h"
#include "io/hBasicIO.h"
#include "exceptions/exceptions.h"
#include "IRQ/irq.h"

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


static IDT* idt;


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
    disable_interrupts();

    idt = ( IDT* )idt_addr;

    init_irq();

    init_exceptions();


    struct {
        uint16_t length;
        uint64_t addr;
    } __attribute__(( packed )) IDTR = { sizeof( IDT ) - 1, idt_addr };

    __asm__ volatile ( "lidt %0" : : "m"( IDTR ) );

    enable_interrupts();
}