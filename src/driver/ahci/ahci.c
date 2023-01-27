#include "ahci.h"
#include "io/io.h"
#include "pci/pci.h"
#include "memory/Paging/hPaging.h"

#define AHCI_PROG_IF 1
#define STORAGE_CLASS 0x1
#define AHCI_SUBCLASS 0x6
#define RAID_SUBCLASS 0x6

#define MAX_AHCI_CONTROLLERS 32
static ahci_controller_t controllers[MAX_AHCI_CONTROLLERS];
static uint32_t controllers_count = 0;

bool init_ahci_controller(pci_dev_t* dev) {
    uint32_t cmd = pci_readd(dev->bus, dev->slot, dev->func, 0x4);
    if ((cmd & (1 << 2)) == 0) {
        cmd = pci_readd(dev->bus, dev->slot, dev->func, 0x4);
        pci_writed(dev->bus, dev->slot, dev->func, 0x4, cmd | (1 << 2));
    }

    if (!is_bar_present(dev, 5)) {
        kprintln("ERROR: cannot locate ahci bar5");
        return false;
    }

    pci_bar_t bar = get_bar(dev, 5);
    kprintln("%x %d", bar.base, bar.size);

    if (bar.size == 0) {
        kprintln("ERROR: ahci controller bar5 size was 0");
        return false;
    }

    for (uint32_t i = 0; i < (bar.size-1) / PAGE_SIZE + 1; i++) {
        map_frame(get_hPage(bar.base + i*PAGE_SIZE), get_hFrame(bar.base + i*PAGE_SIZE), Present | Writeable | DisableCache);
    }

    ahci_regs_t* regs = (ahci_regs_t*)bar.base;

    uint16_t version_maj = (regs->vs >> 16) & 0xffff;
    uint16_t version_min = regs->vs & 0xffff;

    kprintln("ahci controller version: %d.%d", version_maj, version_min);

    if ((regs->cap & (1 << 31)) == 0) {
        kprintln("ERROR: AHCI controller does not support 64bit addressing");
        return false;
    }

    controllers[controllers_count] = (ahci_controller_t) {
        .pci_bar = bar,
        .regs = regs,
        .version_min = version_min,
        .version_maj = version_maj
    };

    return true;
}

void init_ahci(void) {
    pci_dev_t* pci_devs = get_pci_devs();
    uint32_t pci_devs_len = get_pci_devs_len();

    bool found_raid = false;
    for (uint32_t i = 0; i < pci_devs_len; i++) {
        if (pci_devs[i].class == STORAGE_CLASS) {
            if (pci_devs[i].subclass == AHCI_SUBCLASS && pci_devs[i].prog_if == AHCI_PROG_IF) {
                kprintln("found AHCI controller ([%x:%x:%x])", pci_devs[i].bus, pci_devs[i].slot, pci_devs[i].func);
                init_ahci_controller(&pci_devs[i]);
                return;
            }

            if (pci_devs[i].subclass == RAID_SUBCLASS) { found_raid = true; }
        }
    }

    kprintln("WARNING: no AHCI detected");
    if (found_raid) {
        kprintln("disable RAID in BIOS to get an AHCI controller instead of a RAID controller");
    }
}
