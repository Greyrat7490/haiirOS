#include "irq.h"
#include "io/hBasicIO.h"

__attribute__(( interrupt ))
void irq0_handler( struct interrupt_frame* frame ) {
    printf( "." );
    outb( 0x20, 0x20 ); //EOI
}

__attribute__(( interrupt ))
void irq1_handler( struct interrupt_frame* frame ) {
    int i = 0;
    i = inb( 0x60 ); // keyboard port

    printf( "%x", i );

    if( i == 0x1c ) // clear screen when Enter is pressed
        clear_screen();

    outb( 0x20, 0x20 );
}

void init_irq() {
    // remap the PIC --------------------------------------------------------
    // TODO: io_wait()
    outb( 0x20, 0x11 ); // init master PIC ( ICW2 - ICW4 )
    outb( 0xA0, 0x11 ); // init slave PIC

    // ICW2
    outb( 0x21, 0x20 ); // set master PIC offset to 0x20
    outb( 0xA1, 0x28 ); // set slave PIC offset to 0x28

    // ICW3
    outb( 0x21, 0x04 ); // tells this PIC there is a second PIC ( at IRQ2 )
    outb( 0xA1, 0x02 ); // tells this PIC its cascade identity

    // ICW4
    outb( 0x21, 0x01 ); // 8086/88 ( MCS-80/85 ) mode
    outb( 0xA1, 0x01 ); // 8086/88 ( MCS-80/85 ) mode

    // set masks
    outb( 0x21, 0x00 );
    outb( 0xA1, 0x00 );
    // ----------------------------------------------------------------------


    init_gate( 32, ( uint64_t )irq0_handler, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE );
    init_gate( 33, ( uint64_t )irq1_handler, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE );
    // interrupt 34( IRQ 2 ) is used by the two PICs internally -> never raised

    // rest --------------------------------------------
    for ( uint64_t i = 35; i < 256; i++ )
        init_gate( i, ( uint64_t )interrupt_handler, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE );
}