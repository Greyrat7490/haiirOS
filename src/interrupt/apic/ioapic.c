#include "ioapic.h"
#include "boot/acpi.h"
#include "apic.h"
#include "io/io.h"
#include "memory/paging.h"

static madt_io_apic_override_t* overrides[VECTOR_COUNT] = {0};

void ioapic_write_reg(uint64_t ioapic_base, uint8_t offset, uint32_t value) {
    *(volatile uint32_t*)ioapic_base = offset;
    *(volatile uint32_t*)(ioapic_base + 0x10) = value; 
}
 
uint32_t ioapic_read_reg(uint64_t ioapic_base, uint8_t offset) {
    *(volatile uint32_t*)ioapic_base = offset;
    return *(volatile uint32_t*)(ioapic_base + 0x10);
}

static void get_overrides(madt_t* madt) {
    uint32_t entries_size = madt->header.Length - ((uint64_t)madt->entries - (uint64_t)madt);

    uint8_t len = 0;
    for (uint32_t entry_idx = 0; entry_idx < entries_size-1; entry_idx+=len) {
        uint8_t type = madt->entries[entry_idx];
        len = madt->entries[entry_idx+1];

        if (type == MADT_ENTRY_INT_SRC_OVERRIDE) {
            madt_io_apic_override_t* override = (madt_io_apic_override_t*) &madt->entries[entry_idx];
            kprintln("%d -> %d (flags: %d)", override->global_system_int, override->irq_src, override->flags & 0xf);

            if (override->global_system_int >= VECTOR_COUNT) {
                kprintln("ERROR got override with gsi >= VECTOR_COUNT (%d)", override->global_system_int);
                continue;
            }

            overrides[override->global_system_int] = override;
        }
    }
}

void init_io_apic(void) {
    madt_t* madt = get_madt();
    uint32_t entries_size = madt->header.Length - ((uint64_t)madt->entries - (uint64_t)madt);

    get_overrides(madt);

    uint8_t len = 0;
    for (uint32_t entry_idx = 0; entry_idx < entries_size-1; entry_idx+=len) {
        uint8_t type = madt->entries[entry_idx];
        len = madt->entries[entry_idx+1];

        if (type == MADT_ENTRY_IOAPIC) {
            madt_io_apic_t* ioapic = (madt_io_apic_t*) &madt->entries[entry_idx];
            map_frame(to_page(ioapic->addr), to_frame(ioapic->addr), Present | Writeable | DisableCache);

            ioapic_write_reg(ioapic->addr, IOAPICID_OFFSET, ioapic->id);

            uint8_t version = ioapic_read_reg(ioapic->addr, IOAPICVER_OFFSET) & 0xff;
            kprintln("io_apic version %x", version);
            
            uint8_t max_entries = ((ioapic_read_reg(ioapic->addr, IOAPICVER_OFFSET) >> 16) + 1) & 0xff;
            kprintln("max_entries: %d", max_entries);

            uint8_t gsi = ioapic->gsi_base;
            for (uint8_t i = 0; i < max_entries; i++, gsi++) {
                uint64_t entry = ((uint64_t)get_local_apic_id() << IOAPIC_DEST_FIELD_BIT) | IOAPIC_DEST_MODE_PHYSICAL |
                    ((gsi + INTERRUPT_BASE) << IOAPIC_INT_VECTOR_BIT);

                // GSI 0 for external interrupts
                if (gsi == 0) {
                    entry |= IOAPIC_TRIGGER_MODE_EDGE | IOAPIC_POLARITY_HIGH_ACTIVE | IOAPIC_DELIVERY_MODE_EXT_INT;                   
                // identity map legacy ISA interrupts
                } else if (gsi < ISA_INTERRUPT_COUNT) {
                    entry |= IOAPIC_TRIGGER_MODE_EDGE | IOAPIC_POLARITY_HIGH_ACTIVE | IOAPIC_DELIVERY_MODE_FIXED;
                // PCI interrupts (masked per default)
                } else {
                    entry |= IOAPIC_TRIGGER_MODE_LEVEL | IOAPIC_POLARITY_LOW_ACTIVE | IOAPIC_MASKED | IOAPIC_DELIVERY_MODE_FIXED;
                }

                if (overrides[gsi] != 0) {
                    entry &= ~((uint64_t)IOAPIC_VECTOR_MASK);
                    entry |= (overrides[gsi]->irq_src + INTERRUPT_BASE) << IOAPIC_INT_VECTOR_BIT;

                    uint8_t polarity = (overrides[gsi]->flags >> IOAPIC_OVERRIDE_POLARITY_OFFSET) & 3;
                    if (polarity == IOAPIC_OVERRIDE_POLARITY_LOW) {
                        entry &= ~((uint64_t)IOAPIC_POLARITY_MASK);
                        entry |= IOAPIC_POLARITY_LOW_ACTIVE;
                    } else if (polarity == IOAPIC_OVERRIDE_POLARITY_HIGH) {
                        entry &= ~((uint64_t)IOAPIC_POLARITY_MASK);
                        entry |= IOAPIC_POLARITY_HIGH_ACTIVE;
                    }

                    uint8_t trigger_mode = (overrides[gsi]->flags >> IOAPIC_OVERRIDE_TRIGGER_OFFSET) & 3;
                    if (trigger_mode == IOAPIC_OVERRIDE_TRIGGER_LEVEL) {
                        entry &= ~((uint64_t)IOAPIC_TRIGGER_MODE_MASK);
                        entry |= IOAPIC_TRIGGER_MODE_LEVEL;

                    } else if (trigger_mode == IOAPIC_OVERRIDE_TRIGGER_EDGE) {
                        entry &= ~((uint64_t)IOAPIC_TRIGGER_MODE_MASK);
                        entry |= IOAPIC_TRIGGER_MODE_EDGE;
                    }
                }

                ioapic_write_reg(ioapic->addr, IOAPICREDTBL_OFFSET(i), entry);
                ioapic_write_reg(ioapic->addr, IOAPICREDTBL_OFFSET(i)+1, entry >> 32);
            }
        }
    }
}
