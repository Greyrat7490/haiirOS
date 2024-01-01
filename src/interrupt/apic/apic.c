#include "apic.h"
#include "boot/acpi.h"
#include "interrupt/exceptions/exceptions.h"
#include "interrupt/asm.h"
#include "io/io.h"
#include "memory/paging.h"
#include "irq.h"
#include "ioapic.h"


static uint64_t apic_base = 0x0;
static bool x2apic = false;

static void init_timer(void) {
    *(volatile uint32_t*)(apic_base+APIC_LVT_TIMER) = APIC_LVT_MASKED;
    uint32_t config = *(volatile uint32_t*)(apic_base+APIC_LVT_TIMER);
    kprintln("lvt_timer %x", config);

    *(volatile uint32_t*)(apic_base+APIC_DIV_CONFIG) = 0;
    config = *(volatile uint32_t*)(apic_base+APIC_DIV_CONFIG) & 0xf;
    kprintln("div config %x", config);
}

static void init_lvt_lints(void) {
    *(volatile uint32_t*)(apic_base+APIC_LVT_LINT0) = APIC_LVT_MASKED;
    *(volatile uint32_t*)(apic_base+APIC_LVT_LINT1) = APIC_LVT_MASKED;
}

static void init_lvt_err(void) {
    uint32_t config = *(volatile uint32_t*)(apic_base+APIC_ERR_REG) & 0xffffff00;
    *(volatile uint32_t*)(apic_base+APIC_ERR_REG) |= config | 0xfe;
}

static void init_spurious_interrupts(void) {
    uint32_t config = *(volatile uint32_t*)(apic_base+APIC_SVR) & 0xffffff00;
    config |= APIC_ENABLE | 0xff;
    *(volatile uint32_t*)(apic_base+APIC_SVR) = config;
}

uint8_t get_local_apic_id(void) {
    // P6 family and Pentium processors only have 4bits
    return *(volatile uint32_t*)(apic_base+APIC_ID) >> APIC_ID_OFFSET;
}

void show_local_apics(void) {
    madt_t* madt = get_madt();
    uint32_t entries_size = madt->header.Length - ((uint64_t)madt->entries - (uint64_t)madt);
    kprintln("entries_size: %d", entries_size);

    uint8_t len = 0;
    for (uint32_t entry_idx = 0; entry_idx < entries_size-1; entry_idx+=len) {
        uint8_t type = madt->entries[entry_idx];
        len = madt->entries[entry_idx+1];

        if (type == MADT_ENTRY_APIC) {
            madt_local_apic_t* lapic = (madt_local_apic_t*) &madt->entries[entry_idx];
            kprintln("local apic id: %d, processor_id: %d", lapic->apic_id, lapic->processor_id);
        }
    }
}

void init_apic(void) {
    init_exceptions();
    apic_init_irq_handlers();

    apic_base = read_msr(MSR_APIC_BASE) & 0xfffff000;
    write_msr(MSR_APIC_BASE, read_msr(MSR_APIC_BASE) | MSR_APIC_BASE_ENABLE);

    x2apic = (read_msr(MSR_APIC_BASE) & MSR_APIC_BASE_X2APIC_ENABLE) != 0;
    bool xapic_mode = (read_msr(MSR_APIC_BASE) & MSR_APIC_BASE_ENABLE) != 0;
    kprintln("xapic_mode: %b", xapic_mode);
    kprintln("x2apic_mode: %b", x2apic);

    map_frame(to_page(apic_base), to_frame(apic_base), Present | Writeable | DisableCache);

    init_spurious_interrupts();
    init_timer();
    init_lvt_err();
    init_lvt_lints();

    init_io_apic();
}

void apic_eoi(void) {
    *(volatile uint32_t*)(apic_base+APIC_EOI) = 0;
}
