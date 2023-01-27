#include "pci.h"
#include "interrupt/ISR/isr.h"
#include "io/io.h"

#define PIC_CONFIG_ADDR_PORT 0xcf8
#define PIC_CONFIG_DATA_PORT 0xcfc

#define PCI_CONFIG_ENABLE_BIT 31
#define PCI_CONFIG_BUS_BIT 16
#define PCI_CONFIG_SLOT_BIT 11
#define PCI_CONFIG_FUNC_BIT 8

#define PCI_MAX_FUNCS 8
#define PCI_MAX_DEVS 32
#define PCI_MAX_BUSES 256


#define MAX_PCI_DEVICES (PCI_MAX_BUSES * PCI_MAX_DEVS )
static pci_dev_t pci_devs[MAX_PCI_DEVICES];
static uint32_t pci_devs_len = 0;

pci_dev_t* get_pci_devs(void) { return pci_devs; }
uint32_t get_pci_devs_len(void) { return pci_devs_len; }


uint16_t pci_readw(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t addr = 
        ((uint32_t)  bus << PCI_CONFIG_BUS_BIT)     |
        ((uint32_t) slot << PCI_CONFIG_SLOT_BIT)    |
        ((uint32_t) func << PCI_CONFIG_FUNC_BIT)    |
        ((uint32_t)    1 << PCI_CONFIG_ENABLE_BIT)  |
        (offset & 0xfc);    // last 2 bits have to be 0

    outd(PIC_CONFIG_ADDR_PORT, addr);
                                        // get first word
    return ind(PIC_CONFIG_DATA_PORT) >> ((offset & 2) * 8) & 0xffff;
}

static void check_bus(uint8_t bus);

static void check_func(uint8_t bus, uint8_t slot, uint8_t func) {
    uint16_t class = pci_readw(bus, slot, func, 0xa);
    uint8_t base_class = (class >> 8) & 0xff;
    uint8_t sub_class = class & 0xff;

    kprintln("pci: [%x:%x:%x]: %x %x", bus, slot, func, base_class, sub_class);

    // PCI to PCI bridge
    if (base_class == 0x6 && sub_class == 0x4) {
        uint8_t secondaryBus = (pci_readw(bus, slot, func, 0x18) >> 8) & 0xff;
        check_bus(secondaryBus);
    } else {
        pci_devs[pci_devs_len] = (pci_dev_t) {
            .bus=bus,
            .slot=slot,
            .func=func,
            .class=base_class,
            .subclass=sub_class
        };
        pci_devs_len++;
    }
}

static void check_bus(uint8_t bus) {
    for (uint8_t dev = 0; dev < PCI_MAX_DEVS; dev++) {
        for (uint8_t func = 0; func < PCI_MAX_FUNCS; func++) {
            if ((pci_readw(bus, dev, func, 0) & 0xffff) != 0xffff) {
                check_func(bus, dev, func);
            }
        }
    }
}

void init_pci(void) {
    uint16_t header_type = pci_readw(0, 0, 0, 0xc);

    // single PCI host controller
    if ((header_type & 0x80) == 0) {
        check_bus(0);

    // multiple PCI host controller
    } else {
        for (uint8_t func = 0; func < PCI_MAX_FUNCS; func++) {
            check_bus(func);
        }
    }
}
