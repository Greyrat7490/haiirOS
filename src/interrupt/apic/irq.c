#include "irq.h"
#include "interrupt/apic/apic.h"
#include "interrupt/asm.h"
#include "interrupt/idt.h"
#include "io/io.h"

// IRQ0 handler
__attribute__ ((interrupt))
static void timer_handler(struct interrupt_frame* frame) {
    (void)frame;

    kprintf(".");

    apic_eoi();
}

// IRQ1 handler
__attribute__ ((interrupt))
static void keyboard_handler(struct interrupt_frame* frame) {
    (void)frame;

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

    apic_eoi();
}

__attribute__((interrupt))
void apic_interrupt_handler (struct interrupt_frame* frame) {
    kprintln("interrupt!");
    kprintln("  instruction pointer: %x", frame->rip);
    kprintln("  code seg: %x", frame->cs);
    kprintln("  rflags: %d", frame->rflags);
    kprintln("  stack pointer: %x", frame->rsp);
    kprintln("  stack seg: %x", frame->ss);

    apic_eoi();
}

__attribute__((interrupt))
void apic_spurious_interrupt_handler (struct interrupt_frame* frame) {
    (void)frame;
    // do nothing
}

__attribute__((interrupt))
void apic_err_handler (struct interrupt_frame* frame) {
    (void)frame;
    // do nothing
}

void apic_init_irq_handlers(void) {
    // vec 20 - 31 are reserved
    init_gate(32, (uint64_t) timer_handler, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE);
    init_gate(33, (uint64_t) keyboard_handler, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE);

    // rest --------------------------------------------
    for (uint64_t i = 34; i < 256 - 2; i++) {
        init_gate(i, (uint64_t) apic_interrupt_handler, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE);
    }

    init_gate(0xfe, (uint64_t) apic_err_handler, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE);
    init_gate(0xff, (uint64_t) apic_spurious_interrupt_handler, CODE_SEG, GATE_PRESENT | INTERRUPT_GATE, IST_NONE);
}
