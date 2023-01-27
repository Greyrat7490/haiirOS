#include "ahci.h"
#include "io/io.h"
#include "pci/pci.h"

void init_ahci(void) {
    pci_dev_t* pci_devs = get_pci_devs();
    uint32_t pci_devs_len = get_pci_devs_len();

    bool found_raid = false;
    for (uint32_t i = 0; i < pci_devs_len; i++) {
        if (pci_devs[i].class == 0x1) {
            if (pci_devs[i].subclass == 0x6) {
                kprintln("found AHCI controller ([%x:%x:%x])", pci_devs[i].bus, pci_devs[i].slot, pci_devs[i].func);
                return;
            } else if (pci_devs[i].subclass == 0x4) {
                found_raid = true;
            }
        }
    }

    kprintln("WARNING: no AHCI detected");
    if (found_raid) {
        kprintln("disable RAID in BIOS to get an AHCI controller instead of a RAID controller");
    }
}
