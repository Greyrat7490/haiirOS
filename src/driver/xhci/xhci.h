#ifndef XHCI_H_
#define XHCI_H_

#include "pci/pci.h"
#include "types.h"

typedef struct {
    pci_bar_t bar;
    uint8_t cap_len;
    uint64_t cap_regs;
    uint8_t max_ports;
    uint64_t op_regs;
    uint64_t port_regs;
    uint64_t runtime_regs;
    bool context_size_64;
} xhci_controller_t;

void init_xhci(void);

#endif // XHCI_H_
