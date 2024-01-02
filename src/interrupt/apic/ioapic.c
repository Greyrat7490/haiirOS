#include "ioapic.h"
#include "boot/acpi.h"
#include "apic.h"
#include "io/io.h"
#include "memory/paging.h"

static ioapic_t ioapics[MAX_IOAPICS] = {0};
static uint8_t ioapics_len = 0;

static madt_io_apic_override_t* overrides[ISA_INTERRUPT_COUNT] = {0};

static bool allocated_io_interrupts[IO_VECTOR_COUNT] = {0};
static io_handler_t io_handlers[IO_VECTOR_COUNT] = {0};

static void ioapic_write_reg(uint64_t ioapic_base, uint8_t offset, uint32_t value) {
    *(volatile uint32_t*)ioapic_base = offset;
    *(volatile uint32_t*)(ioapic_base + 0x10) = value; 
}
 
static uint32_t ioapic_read_reg(uint64_t ioapic_base, uint8_t offset) {
    *(volatile uint32_t*)ioapic_base = offset;
    return *(volatile uint32_t*)(ioapic_base + 0x10);
}

static void ioapic_write_entry(ioapic_t* ioapic, uint8_t entry_idx, uint64_t entry) {
    ioapic_write_reg(ioapic->addr, IOAPICREDTBL_OFFSET(entry_idx), entry);
    ioapic_write_reg(ioapic->addr, IOAPICREDTBL_OFFSET(entry_idx)+1, entry >> 32);
}

static uint64_t ioapic_read_entry(ioapic_t* ioapic, uint8_t entry_idx) {
    uint64_t low = ioapic_read_reg(ioapic->addr, IOAPICREDTBL_OFFSET(entry_idx));
    uint64_t high = ioapic_read_reg(ioapic->addr, IOAPICREDTBL_OFFSET(entry_idx)+1);
    return (high << 32) | low;
}

static int32_t io_handler_source_override(void* data) {
    uint8_t vector = (uint64_t)data;
    return exec_io_handler(vector);
}

static void init_overrides(madt_t* madt) {
    uint32_t entries_size = madt->header.Length - ((uint64_t)madt->entries - (uint64_t)madt);

    uint8_t len = 0;
    for (uint32_t entry_idx = 0; entry_idx < entries_size-1; entry_idx+=len) {
        uint8_t type = madt->entries[entry_idx];
        len = madt->entries[entry_idx+1];

        if (type == MADT_ENTRY_INT_SRC_OVERRIDE) {
            madt_io_apic_override_t* override = (madt_io_apic_override_t*) &madt->entries[entry_idx];
            kprintln("%d -> %d (flags: %d)", override->global_system_int, override->irq_src, override->flags & 0xf);

            if (override->global_system_int >= ISA_INTERRUPT_COUNT) {
                kprintln("ERROR got override with gsi >= ISA_INTERRUPT_COUNT (%d)", override->global_system_int);
                continue;
            }

            overrides[override->irq_src] = override;

            install_io_interrupt_handler(override->global_system_int, io_handler_source_override, (void*)(uint64_t)override->irq_src);
        }
    }
}

static uint64_t override_entry(uint64_t entry, uint8_t srcIrq) {
    if (overrides[srcIrq] != 0) {
        uint8_t polarity = (overrides[srcIrq]->flags >> IOAPIC_OVERRIDE_POLARITY_OFFSET) & 3;
        if (polarity == IOAPIC_OVERRIDE_POLARITY_LOW) {
            entry &= ~((uint64_t)IOAPIC_POLARITY_MASK);
            entry |= IOAPIC_POLARITY_LOW_ACTIVE;
        } else if (polarity == IOAPIC_OVERRIDE_POLARITY_HIGH) {
            entry &= ~((uint64_t)IOAPIC_POLARITY_MASK);
            entry |= IOAPIC_POLARITY_HIGH_ACTIVE;
        }

        uint8_t trigger_mode = (overrides[srcIrq]->flags >> IOAPIC_OVERRIDE_TRIGGER_OFFSET) & 3;
        if (trigger_mode == IOAPIC_OVERRIDE_TRIGGER_LEVEL) {
            entry &= ~((uint64_t)IOAPIC_TRIGGER_MODE_MASK);
            entry |= IOAPIC_TRIGGER_MODE_LEVEL;

        } else if (trigger_mode == IOAPIC_OVERRIDE_TRIGGER_EDGE) {
            entry &= ~((uint64_t)IOAPIC_TRIGGER_MODE_MASK);
            entry |= IOAPIC_TRIGGER_MODE_EDGE;
        }
    }

    return entry;
}

static void get_io_apics(madt_t* madt) {
    uint32_t entries_size = madt->header.Length - ((uint64_t)madt->entries - (uint64_t)madt);
    uint8_t len = 0;

    for (uint32_t entry_idx = 0; entry_idx < entries_size-1; entry_idx+=len) {
        uint8_t type = madt->entries[entry_idx];
        len = madt->entries[entry_idx+1];

        if (type == MADT_ENTRY_IOAPIC) {
            madt_io_apic_t* ioapic = (madt_io_apic_t*) &madt->entries[entry_idx];

            map_frame(to_page(ioapic->addr), to_frame(ioapic->addr), Present | Writeable | DisableCache);
            ioapic_write_reg(ioapic->addr, IOAPICID_OFFSET, ioapic->id);

            uint8_t version = ioapic_read_reg(ioapic->addr, IOAPICVER_OFFSET) & 0xff;
            uint8_t max_entries = ((ioapic_read_reg(ioapic->addr, IOAPICVER_OFFSET) >> 16) + 1) & 0xff;

            kprintln("io_apic version %x", version);
            kprintln("max_entries: %d", max_entries);

            ioapics[ioapics_len++] = (ioapic_t) { 
                .gsi_base = ioapic->gsi_base,
                .max_entries = max_entries,
                .version = version,
                .addr = ioapic->addr,
                .id = ioapic->id
            };
        }
    }
}

void enable_io_interrupt(uint8_t gsi) {
    if (gsi < ISA_INTERRUPT_COUNT && overrides[gsi] != 0x0) {
        gsi = overrides[gsi]->global_system_int;
    }

    ioapic_t* ioapic = get_io_apic(gsi);
    uint8_t entry_idx = gsi - ioapic->gsi_base;

    uint64_t entry = ioapic_read_entry(ioapic, entry_idx);
    entry &= ~((uint64_t)IOAPIC_MASKED);
    ioapic_write_entry(ioapic, entry_idx, entry);
}

void disable_io_interrupt(uint8_t gsi) {
    ioapic_t* ioapic = get_io_apic(gsi);
    uint8_t entry_idx = gsi - ioapic->gsi_base;

    uint64_t entry = ioapic_read_entry(ioapic, entry_idx);
    entry |= IOAPIC_MASKED;
    ioapic_write_entry(ioapic, entry_idx, entry);
}

ioapic_t* get_io_apic(uint8_t gsi) {
    for (uint8_t i = 0; i < ioapics_len; i++) {
        if (ioapics[i].gsi_base <= gsi && gsi < ioapics[i].max_entries + ioapics[i].gsi_base) {
            return &ioapics[i];
        }
    }

    return 0x0;
}

int32_t alloc_io_interrupts(uint8_t count, uint8_t* firstVector) {
    for (uint32_t i = 0; i < IO_VECTOR_COUNT; i++) {
        if (allocated_io_interrupts[i]) { continue; }

        bool found = true;
        for (uint32_t j = 0; j < count; j++) {
            if (allocated_io_interrupts[i+j]) { 
                i += j;
                found = false;
            }
        }

        if (found) {
            for (uint32_t j = 0; j < count; j++) {
                allocated_io_interrupts[i+j] = true;
                reserve_io_interrupt(i+j);
            }

            *firstVector = i;
            return 0;
        }
    }

    return -1;
}

void reserve_io_interrupt(uint8_t vector) {
    if (vector >= IO_VECTOR_COUNT) {
        kprintln("ERROR invalid vector (%d >= %d)", vector, IO_VECTOR_COUNT);
    }

    if (allocated_io_interrupts[vector]) {
        kprintln("WARNING: io interrupt (%d) is already free", vector);
    }

    allocated_io_interrupts[vector] = true;
}

void free_io_interrupt(uint8_t vector) {
    if (vector >= IO_VECTOR_COUNT) {
        kprintln("ERROR invalid vector (%d >= %d)", vector, IO_VECTOR_COUNT);
    }

    if (!allocated_io_interrupts[vector]) {
        kprintln("WARNING: io interrupt (%d) is already free", vector);
    }

    allocated_io_interrupts[vector] = false;
}

int32_t install_io_interrupt_handler(uint8_t vector, interrupt_handler_t handler, void* data) {
    if (vector >= IO_VECTOR_COUNT) {
        kprintln("ERROR invalid vector (%d >= %d)", vector, IO_VECTOR_COUNT);
        return -1;
    }

    io_handler_t io_handler = { .vector = vector, .func = handler, .data = data };

    if (io_handlers[vector].func == 0x0 && io_handlers[vector].vector == 0 && io_handlers[vector].data == 0x0) {
        io_handlers[vector] = io_handler;
        enable_io_interrupt(vector);
    } else {
        kprintln("ERROR vector %d already has an installed handler", vector);
        return -1;
    }

    return 0;
}

int32_t remove_io_interrupt_handler(uint8_t vector) {
    if (vector >= IO_VECTOR_COUNT) {
        kprintln("ERROR invalid vector (%d >= %d)", vector, IO_VECTOR_COUNT);
        return -1;
    }

    io_handlers[vector] = (io_handler_t){ .vector = 0, .func = 0x0, .data = 0x0 };

    disable_io_interrupt(vector);
    return 0;
}

int32_t exec_io_handler(uint8_t vector) {
    if (io_handlers[vector].func == 0x0) {
        kprintln("ERROR: cannot call handler of io interrupt %d (handler is not defined)", vector);
    }

    return io_handlers[vector].func(io_handlers[vector].data);
}

void init_io_apics(void) {
    madt_t* madt = get_madt();

    get_io_apics(madt);

    init_overrides(madt);

    for (uint32_t i = 0; i < ioapics_len; i++) {
        uint8_t gsi = ioapics[i].gsi_base;
        for (uint8_t entry_idx = 0; entry_idx < ioapics[i].max_entries; entry_idx++, gsi++) {
            // masked per default
            uint64_t entry = ((uint64_t)get_local_apic_id() << IOAPIC_DEST_FIELD_BIT) | IOAPIC_MASKED | IOAPIC_DEST_MODE_PHYSICAL |
                ((gsi + INTERRUPT_BASE) << IOAPIC_INT_VECTOR_BIT);

            // GSI 0 for external interrupts
            if (gsi == 0) {
                entry |= IOAPIC_TRIGGER_MODE_EDGE | IOAPIC_POLARITY_HIGH_ACTIVE | IOAPIC_DELIVERY_MODE_EXT_INT;                   
            // identity map legacy ISA interrupts
            } else if (gsi < ISA_INTERRUPT_COUNT) {
                entry |= IOAPIC_TRIGGER_MODE_EDGE | IOAPIC_POLARITY_HIGH_ACTIVE | IOAPIC_DELIVERY_MODE_FIXED;
                entry = override_entry(entry, gsi);
            // PCI interrupts
            } else {
                entry |= IOAPIC_TRIGGER_MODE_LEVEL | IOAPIC_POLARITY_LOW_ACTIVE | IOAPIC_DELIVERY_MODE_FIXED;
            }

            ioapic_write_entry(&ioapics[i], entry_idx, entry);

            reserve_io_interrupt(gsi);
        }
    }
}
