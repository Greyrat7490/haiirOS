#ifndef APIC_H_
#define APIC_H_

#include "types.h"

#define MSR_APIC_BASE 0x1b
#define MSR_APIC_BASE_ENABLE (1 << 11)
#define MSR_APIC_BASE_X2APIC_ENABLE (1 << 10)

#define APIC_ID 0x20
#define APIC_ID_OFFSET 24
#define APIC_EOI 0xb0
#define APIC_DEST_FORMAT 0xe0
#define APIC_SVR 0xf0
#define APIC_PRIORITY 0x80
#define APIC_ENABLE (1 << 8)
#define APIC_LVT_TIMER 0x320
#define APIC_LVT_LINT0 0x350
#define APIC_LVT_LINT1 0x360
#define APIC_ERR_REG 0x370
#define APIC_DIV_CONFIG 0x3e0
#define APIC_LVT_MASKED (1 << 16)

#define ISA_INTERRUPT_COUNT 0x10
#define INTERRUPT_BASE 0x20
#define IO_VECTOR_COUNT (256 - INTERRUPT_BASE)

void init_apic(void);
void apic_eoi(void);
uint8_t get_local_apic_id(void);
void show_local_apics(void);

#endif // APIC_H_
