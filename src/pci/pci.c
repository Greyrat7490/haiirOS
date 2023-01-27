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
    return pci_readd(bus, slot, func, offset) & 0xffff;
}

uint32_t pci_readd(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t addr =
        ((uint32_t)  bus << PCI_CONFIG_BUS_BIT)     |
        ((uint32_t) slot << PCI_CONFIG_SLOT_BIT)    |
        ((uint32_t) func << PCI_CONFIG_FUNC_BIT)    |
        ((uint32_t)    1 << PCI_CONFIG_ENABLE_BIT)  |
        (offset & 0xfc);    // last 2 bits have to be 0

    outd(PIC_CONFIG_ADDR_PORT, addr);
                                        // get first word
    return ind(PIC_CONFIG_DATA_PORT) >> ((offset & 2) * 8);
}

void pci_writed(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value) {
    uint32_t addr =
        ((uint32_t)  bus << PCI_CONFIG_BUS_BIT)     |
        ((uint32_t) slot << PCI_CONFIG_SLOT_BIT)    |
        ((uint32_t) func << PCI_CONFIG_FUNC_BIT)    |
        ((uint32_t)    1 << PCI_CONFIG_ENABLE_BIT)  |
        (offset & 0xfc);    // last 2 bits have to be 0

    outd(PIC_CONFIG_ADDR_PORT, addr);
    outd(PIC_CONFIG_DATA_PORT + (offset & 3), value);
}

bool is_bar_present(pci_dev_t* dev, uint8_t bar) {
    uint32_t offset = 0x10 + bar*4;
    return pci_readd(dev->bus, dev->slot, dev->func, offset) != 0;
}

pci_bar_t get_bar(pci_dev_t* dev, uint8_t bar) {
    uint32_t offset = 0x10 + bar*4;
    uint32_t base_low = pci_readd(dev->bus, dev->slot, dev->func, offset);

    bool is_mmio = (base_low & 1) == 0;
    bool is_prefetchable = is_mmio && (base_low & (1 << 3));
    bool is_64bits = is_mmio && ((base_low & 6) == 2);
    uint32_t base_high = 0;
    if (is_64bits) {
        base_high = pci_readd(dev->bus, dev->slot, dev->func, offset + 4);
    }

    uint64_t base = ((uint64_t)base_high << 32 | base_low);
    if (is_mmio) {
        base &= ~0xf;
    } else {
        base &= ~0xd;
    }

    pci_writed(dev->bus, dev->slot, dev->func, offset, (uint32_t)-1);
    uint32_t size_low = pci_readd(dev->bus, dev->slot, dev->func, offset);
    pci_writed(dev->bus, dev->slot, dev->func, offset, base_low);

    uint32_t size_high;
    if (is_64bits) {
        pci_writed(dev->bus, dev->slot, dev->func, offset+4, (uint32_t)-1);
        size_high = pci_readd(dev->bus, dev->slot, dev->func, offset+4);
        pci_writed(dev->bus, dev->slot, dev->func, offset+4, base_high);
    } else {
        size_high = (uint32_t)-1;
    }

    uint64_t size = ((uint64_t)size_high << 32 | size_low);
    if (is_mmio) {
        size &= ~0xf;
    } else {
        size &= ~0xd;
    }

    size = ~size + 1;

    return (pci_bar_t) {
        .base = base,
        .size = size,
        .is_mmio = is_mmio,
        .is_prefetchable = is_prefetchable
    };
}

static void check_bus(uint8_t bus);

static void check_func(uint8_t bus, uint8_t slot, uint8_t func) {
    uint16_t class = pci_readw(bus, slot, func, 0xa);
    uint8_t base_class = (class >> 8) & 0xff;
    uint8_t sub_class = class & 0xff;
    uint8_t prog_if = (pci_readw(bus, slot, func, 0x8) >> 8) & 0xff;

    kprintln("pci: [%x:%x:%x]: %x %x", bus, slot, func, base_class, sub_class);

    // PCI to PCI bridge
    if (base_class == 0x6 && sub_class == 0x4) {
        uint8_t secondaryBus = (pci_readw(bus, slot, func, 0x18) >> 8) & 0xff;
        check_bus(secondaryBus);
    } else {
        pci_devs[pci_devs_len] = (pci_dev_t) {
            .bus = bus,
            .slot = slot,
            .func = func,
            .class = base_class,
            .subclass = sub_class,
            .prog_if = prog_if
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
