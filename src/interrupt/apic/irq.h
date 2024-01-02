#ifndef IRQ_APIC_H_
#define IRQ_APIC_H_

#include "types.h"

typedef int32_t (*interrupt_handler_t)(void* data);

void apic_init_irq_handlers(void);

#endif // IRQ_APIC_H_
