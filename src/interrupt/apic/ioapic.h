#ifndef IO_APIC_H_
#define IO_APIC_H_

#include "types.h"
#include "interrupt/apic/irq.h"

#define IOAPICID_OFFSET 0x0
#define IOAPICVER_OFFSET 0x1
#define IOAPICARB_OFFSET 0x2
#define IOAPICREDTBL_OFFSET(idx) (0x10 + 2*idx)

#define IOAPIC_INT_VECTOR_BIT 0
#define IOAPIC_DELIVERY_MODE_BIT 8
#define IOAPIC_DEST_MODE_BIT 11
#define IOAPIC_DELIVERY_STATUS_BIT 12
#define IOAPIC_POLARITY_MODE_BIT 13
#define IOAPIC_IRR_MODE_BIT 14
#define IOAPIC_TRIGGER_MODE_BIT 15
#define IOAPIC_MASK_BIT 16
#define IOAPIC_DEST_FIELD_BIT 56

#define IOAPIC_VECTOR_MASK (0xff << IOAPIC_INT_VECTOR_BIT)
#define IOAPIC_DELIVERY_MODE_MASK (7 << IOAPIC_DELIVERY_MODE_BIT)
#define IOAPIC_POLARITY_MASK (1 << IOAPIC_POLARITY_MODE_BIT)
#define IOAPIC_TRIGGER_MODE_MASK (1 << IOAPIC_TRIGGER_MODE_BIT)

#define IOAPIC_DELIVERY_MODE_FIXED 0
#define IOAPIC_DELIVERY_MODE_LOWEST_PRIO (1 << 8)
#define IOAPIC_DELIVERY_MODE_SMI (2 << 8)
#define IOAPIC_DELIVERY_MODE_NMI (4 << 8)
#define IOAPIC_DELIVERY_MODE_INIT (5 << 8)
#define IOAPIC_DELIVERY_MODE_EXT_INT (7 << 8)
#define IOAPIC_DELIVERY_STATUS_PENDING (1 << 12)
#define IOAPIC_POLARITY_HIGH_ACTIVE 0
#define IOAPIC_POLARITY_LOW_ACTIVE (1 << 13)
#define IOAPIC_DEST_MODE_PHYSICAL 0
#define IOAPIC_DEST_MODE_LOGICAL IOAPIC_DEST_MODE_BIT
#define IOAPIC_TRIGGER_MODE_EDGE 0
#define IOAPIC_TRIGGER_MODE_LEVEL (1 << 15)
#define IOAPIC_MASKED (1 << 16)

#define IOAPIC_OVERRIDE_POLARITY_OFFSET 0
#define IOAPIC_OVERRIDE_TRIGGER_OFFSET 2
#define IOAPIC_OVERRIDE_POLARITY_HIGH 0
#define IOAPIC_OVERRIDE_POLARITY_LOW 3
#define IOAPIC_OVERRIDE_TRIGGER_EDGE 0
#define IOAPIC_OVERRIDE_TRIGGER_LEVEL 3

#define MAX_IOAPICS 0xff

typedef struct {
    uint32_t addr;
    uint32_t gsi_base;
    uint8_t id;
    uint8_t max_entries;
    uint8_t version;
} ioapic_t;


typedef struct {
    interrupt_handler_t func;
    void* data;
    uint8_t vector;
} io_handler_t;


void init_io_apics(void);

int32_t install_io_interrupt_handler(uint8_t vector, interrupt_handler_t handler, void* data);
int32_t remove_io_interrupt_handler(uint8_t vector);
void reserve_io_interrupt(uint8_t gsi);
void free_io_interrupt(uint8_t gsi);
void enable_io_interrupt(uint8_t gsi);
void disable_io_interrupt(uint8_t gsi);
int32_t alloc_io_interrupts(uint8_t count, uint8_t* firstVector);

ioapic_t* get_io_apic(uint8_t gsi);

int32_t exec_io_handler(uint8_t vector);

#endif // IO_APIC_H_
