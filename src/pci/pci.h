#ifndef PCI_H_
#define PCI_H_

#include "types.h"

#define STORAGE_CLASS 0x1
#define SERIAL_BUS_CLASS 0xc

#define AHCI_SUBCLASS 0x6
#define USB_SUBCLASS 0x3
#define RAID_SUBCLASS 0x4

#define AHCI_PROG_IF 1
#define XHCI_PROG_IF 0x30

#define PCI_COMMAND_OFFSET 0x4

#define PCI_CMD_MASTER      (1 << 2)
#define PCI_CMD_MEMORY      (1 << 1)
#define PCI_CMD_IO          (1 << 0)
#define PCI_CMD_INT_DISABLE (1 << 10)


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
    bool is_64bits;
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

uint8_t pci_msi_vec_count(pci_dev_t* dev);

#endif // PCI_H_
