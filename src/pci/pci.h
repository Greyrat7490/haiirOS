#ifndef PCI_H_
#define PCI_H_

#include "types.h"

typedef struct {
    uint8_t bus;
    uint8_t slot;
    uint8_t func;
    uint8_t class;
    uint8_t subclass;
    uint8_t prog_if;
} pci_dev_t;

typedef struct {
    uint64_t base;
    uint64_t size;
    bool is_prefetchable;
    bool is_mmio;
} pci_bar_t;

void init_pci(void);

uint16_t pci_readw(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint32_t pci_readd(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void pci_writed(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value);

bool is_bar_present(pci_dev_t* dev, uint8_t bar);
pci_bar_t get_bar(pci_dev_t* dev, uint8_t bar);

pci_dev_t* get_pci_devs(void);
uint32_t get_pci_devs_len(void);

#endif // PCI_H_
