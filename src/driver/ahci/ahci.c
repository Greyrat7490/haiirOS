#include "ahci.h"
#include "io/io.h"
#include "pci/pci.h"
#include "memory/Paging/hPaging.h"
#include "interrupt/ISR/isr.h"

#define AHCI_PROG_IF 1
#define STORAGE_CLASS 0x1
#define AHCI_SUBCLASS 0x6
#define RAID_SUBCLASS 0x4

#define SATA_SIG_ATA    0x101
#define SATA_SIG_ATAPI  0xeb140101
#define SATA_SIG_PM     0x96690101  // port multiplier
#define SATA_SIG_SEMB   0xc33c0101  // enclosure management bridge

#define AHCI_DEV_SATA   1
#define AHCI_DEV_SATAPI 2
#define AHCI_DEV_SEMB   3
#define AHCI_DEV_PM     4

#define HBA_PORT_IPM_ACTIVE 1
#define HBA_PORT_DET_PRESENT 3


#define MAX_AHCI_CONTROLLERS 32
static ahci_controller_t controllers[MAX_AHCI_CONTROLLERS];
static uint32_t controllers_count = 0;

uint8_t get_ahci_dev_type(ahci_port_regs_t* port) {
    uint32_t ssts = port->ssts;
    uint8_t det = ssts & 0xf;
    uint8_t ipm = (ssts >> 8) & 0xf;

    if (det != HBA_PORT_DET_PRESENT || ipm != HBA_PORT_IPM_ACTIVE) {
        return -1;
    }

    // getting device kind from sig register is not reliable (some AHCI controller are buggy)
    // get identify data read back from device
    switch (port->sig) {
    case SATA_SIG_ATAPI:
        return AHCI_DEV_SATAPI;
    case SATA_SIG_PM:
        return AHCI_DEV_PM;
    case SATA_SIG_SEMB:
        return AHCI_DEV_SEMB;
    default:
        return AHCI_DEV_SATA;
    }
}

bool bios_handoff(volatile ahci_regs_t* regs) {
    if ((regs->cap2 & 1) == 0) {
        kprintln("WARNING: AHCI controller does not support bios handoff");     // not needed on all hardware?
        return false;
    }

    regs->bohc |= 1 << 1;

    while ((regs->bohc & 1) == 0) {
        __asm__ volatile ("pause");
    }

    for (uint32_t i = 0; i < 25 * 1000; i++) { io_wait(); } // wait ~25ms

    if ((regs->bohc & (1 << 4)) != 0) {
        for (uint32_t i = 0; i < 2 * 1000 * 1000; i++) { io_wait(); } // wait ~2s
    }

            // bit 4 or 0
    if ((regs->bohc & 11) != 0 || (regs->bohc & (1 << 1)) == 0) {
        kprintln("ERROR: AHCI controller bios handoff failed");
        return false;
    }

    return true;
}

uint32_t find_cmd_slot(uint32_t cmd_slots, ahci_port_regs_t* port) {
    for (uint32_t i = 0; i < cmd_slots; i++) {
        if (((port->ci | port->sact) & (1 << i)) == 0) {
            return i;
        }
    }

    return (uint32_t)-1;
}

ahci_dev_t init_ahci_dev(uint32_t cmd_slots, ahci_port_regs_t* port) {
    uint32_t cmd_slot = find_cmd_slot(cmd_slots, port);
    if (cmd_slot == (uint32_t)-1) {
        kprintln("ERROR: no free command slot (retry later)");
        return (ahci_dev_t){0};
    }

    return (ahci_dev_t) {
        .regs=port
    };
}

bool init_ahci_controller(pci_dev_t* dev) {
    uint32_t cmd = pci_readd(dev->bus, dev->slot, dev->func, 0x4);
    if ((cmd & (1 << 2)) == 0) {
        cmd = pci_readd(dev->bus, dev->slot, dev->func, 0x4);
        pci_writed(dev->bus, dev->slot, dev->func, 0x4, cmd | (1 << 2));
    }

    if (!is_bar_present(dev, 5)) {
        kprintln("ERROR: cannot locate AHCI BAR5");
        return false;
    }

    pci_bar_t bar = get_bar(dev, 5);
    kprintln("%x %d", bar.base, bar.size);

    if (bar.size == 0) {
        kprintln("ERROR: AHCI controller BAR5 size was 0");
        return false;
    }

    for (uint32_t i = 0; i < (bar.size-1) / PAGE_SIZE + 1; i++) {
        map_frame(get_hPage(bar.base + i*PAGE_SIZE), get_hFrame(bar.base + i*PAGE_SIZE), Present | Writeable | DisableCache);
    }

    ahci_regs_t* regs = (ahci_regs_t*)bar.base;

    uint16_t version_maj = (regs->vs >> 16) & 0xffff;
    uint16_t version_min = regs->vs & 0xffff;

    kprintln("AHCI controller version: %d.%d", version_maj, version_min);

    if ((regs->cap & (1 << 31)) == 0) {
        kprintln("ERROR: AHCI controller does not support 64bit addressing");
        return false;
    }

    if (bios_handoff(regs)) {
        kprintln("handed off AHCI controller from BIOS");
    }

    regs->ghc |= (1 << 31);
    regs->ghc &= ~(1 << 1);

    uint32_t ports_count = regs->cap & 0x1f;
    uint32_t cmd_slots = (regs->cap >> 8) & 0x1f;

    kprintln("ports_count: %d", ports_count);
    kprintln("cmd_slots: %d", cmd_slots);

    ahci_controller_t controller = {
        .devs = {{0}},
        .devs_count = 0,
        .pci_bar = bar,
        .regs = regs,
        .cmd_slots = cmd_slots,
        .ports_count = ports_count,
        .version_min = version_min,
        .version_maj = version_maj
    };

    for (uint32_t i = 0; i < ports_count; i++) {
        if ((regs->pi & (1 << i)) != 0) {
            ahci_port_regs_t* port = (ahci_port_regs_t*)(bar.base + sizeof(ahci_regs_t) + i * sizeof(ahci_port_regs_t));

            uint8_t dev_type = get_ahci_dev_type(port);

            switch (dev_type) {
            case AHCI_DEV_SATA:
                kprintln("found SATA device on port %d", i);
                controller.devs[controller.devs_count++] = init_ahci_dev(cmd_slots, port);
                break;

            case AHCI_DEV_SATAPI:
                kprintln("found SATAPI device on port %d", i);
                break;

            case AHCI_DEV_PM:
                kprintln("found PM device on port %d", i);
                break;

            case AHCI_DEV_SEMB:
                kprintln("found SEMB device on port %d", i);
            }
        }
    }

    controllers[controllers_count++] = controller;
    return true;
}

void init_ahci(void) {
    pci_dev_t* pci_devs = get_pci_devs();
    uint32_t pci_devs_len = get_pci_devs_len();

    bool found_raid = false;
    bool found_ahci = false;
    for (uint32_t i = 0; i < pci_devs_len; i++) {
        if (pci_devs[i].class == STORAGE_CLASS) {
            if (pci_devs[i].subclass == AHCI_SUBCLASS && pci_devs[i].prog_if == AHCI_PROG_IF) {
                kprintln("found AHCI controller ([%x:%x:%x])", pci_devs[i].bus, pci_devs[i].slot, pci_devs[i].func);
                init_ahci_controller(&pci_devs[i]);
                found_ahci = true;
                continue;
            }

            if (pci_devs[i].subclass == RAID_SUBCLASS) { found_raid = true; }
        }
    }

    if (!found_ahci) {
        kprintln("WARNING: no AHCI detected");
        if (found_raid) {
            kprintln("disable RAID in BIOS to get an AHCI controller instead of a RAID controller");
        }
    }
}
