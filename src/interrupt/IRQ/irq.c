#include "irq.h"
#include "io/io.h"
#include "io/input.h"
#include "interrupt/idt.h"
#include "interrupt/ISR/isr.h"


// IRQ0 handler
__attribute__ ((interrupt))
static void timer_handler(struct interrupt_frame* frame) {
    (void)frame;

    outb(0x20, 0x20); // end of interrupt (EOI)
}

// IRQ1 handler
__attribute__ ((interrupt))
static void keyboard_handler(struct interrupt_frame* frame) {
    (void)frame;

    disable_interrupts();

    static uint8_t seq = 0;

    int scancode = inb(0x60);
    enum keystate state;

    if (scancode == 0xe0)
        seq = 1;
    else {
        if(scancode > 0x80)
            state = KEY_RELEASED;
        else
            state = KEY_PRESSED;

        if(seq == 1) {
            scancode += 0x80;
            seq = 0;
        }

        if (state == KEY_PRESSED) {
            kset_color(BLACK, CYAN);
            kprintf("%c", scancode_to_ascii(scancode));
        }
    }

    outb(0x20, 0x20);

    enable_interrupts();
}

void init_irq(void) {
    remap_pic();

    init_gate(32, (uint64_t )timer_handler, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE);
    init_gate(33, (uint64_t) keyboard_handler, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE);
    // interrupt 34 (IRQ 2) is used by the two PICs internally -> never raised

    // rest --------------------------------------------
    for (uint64_t i = 35; i < 256; i++)
        init_gate(i, (uint64_t) interrupt_handler, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE);
}
