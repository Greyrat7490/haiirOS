#include "idt.h"
#include "interrupt/asm.h"
#include "io/io.h"
#include "pic/pic.h"
#include "cpuid/cpuid.h"
#include "apic/apic.h"

struct IDT_descr {
    uint16_t offset1;
    uint16_t selector;
    uint8_t ist;
    uint8_t type;
    uint16_t offset2;
    uint32_t offset3;
    uint32_t zero;
};

typedef struct {
    struct IDT_descr entries[256];
} IDT;

static IDT idt;


void init_gate(uint8_t idt_index, uint64_t base, uint16_t selector, uint8_t type, uint8_t ist) {
    idt.entries[idt_index].zero = 0;

    idt.entries[idt_index].offset1 = (base & 0xffff);
    idt.entries[idt_index].offset2 = ((base >> 16) & 0xffff);
    idt.entries[idt_index].offset3 = ((base >> 32) & 0xffffffff);

    idt.entries[idt_index].selector = selector;
    idt.entries[idt_index].type = type;
    idt.entries[idt_index].ist = ist;
}

void init_interrupts(void) {
    disable_interrupts();

    kprintln("IDT at: %x", (uint64_t)&idt);

    struct {
        uint16_t length;
        uint64_t addr;
    } __attribute__((packed)) IDTR = { sizeof(IDT) - 1, (uint64_t) &idt };

    __asm__ ("lidt %0" : : "m"(IDTR));

    if (cpuid_apic_available()) {
        disable_pic();
        init_apic();
    } else {
        kprintln("WARNING: could not detect APIC (using PIC instead)");
        init_pic();
    }

    enable_interrupts();
}
