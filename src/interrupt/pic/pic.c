#include "interrupt/asm.h"
#include "interrupt/exceptions/exceptions.h"
#include "io/io.h"
#include "io/input.h"
#include "interrupt/idt.h"


// IRQ0 handler
__attribute__ ((interrupt))
static void timer_handler(struct interrupt_frame* frame) {
    (void)frame;

    kprintf(".");
    
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

__attribute__((interrupt))
void interrupt_handler (struct interrupt_frame* frame) {
    kprintln("interrupt!");
    kprintln("  instruction pointer: %x", frame->rip);
    kprintln("  code seg: %x", frame->cs);
    kprintln("  rflags: %d", frame->rflags);
    kprintln("  stack pointer: %x", frame->rsp);
    kprintln("  stack seg: %x", frame->ss);
    outb(0x20, 0x20);
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
    outb(0x20, 0x20);
}

void init_irq_handlers(void) {
    // vec 20 - 31 are reserved
    init_gate(32, (uint64_t) timer_handler, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE);
    init_gate(33, (uint64_t) keyboard_handler, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE);
    // interrupt 34 (IRQ 2) is used by the two PICs internally -> never raised

    // rest --------------------------------------------
    for (uint64_t i = 35; i < 256; i++)
        init_gate(i, (uint64_t) interrupt_handler, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE);
}

void remap_pic(void) {
    disable_interrupts();

    // remap the PIC --------------------------------------------------------
    io_wait();
    outb(0x20, 0x11); // init master PIC (ICW2 - ICW4)
    outb(0xa0, 0x11); // init slave PIC

    // ICW2
    outb(0x21, 0x20); // set master PIC offset to 0x20
    outb(0xa1, 0x28); // set slave PIC offset to 0x28

    // ICW3
    outb(0x21, 0x04); // tells this PIC there is a second PIC (at IRQ2)
    outb(0xa1, 0x02); // tells this PIC its cascade identity

    // ICW4
    outb(0x21, 0x01); // 8086/88 (MCS-80/85) mode
    outb(0xa1, 0x01); // 8086/88 (MCS-80/85) mode

    // set masks
    outb(0x21, 0x00);
    outb(0xa1, 0x00);
    // ----------------------------------------------------------------------

    enable_interrupts();
}

void disable_pic(void) {
    outb(0xa1, 0xff);
    outb(0x21, 0xff);
}

void init_pic(void) {
    init_irq_handlers();
    init_exceptions();
    remap_pic();
}

