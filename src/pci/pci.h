#ifndef PCI_H_
#define PCI_H_

#include "types.h"

typedef struct {
    uint8_t bus;
    uint8_t slot;
    uint8_t func;
    uint8_t class;
    uint8_t subclass;
} pci_dev_t;

void init_pci(void);
uint16_t pci_readw(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
pci_dev_t* get_pci_devs(void);
uint32_t get_pci_devs_len(void);

#endif // PCI_H_
