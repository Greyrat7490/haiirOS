#ifndef AHCI_H_
#define AHCI_H_

#include "types.h"
#include "pci/pci.h"

typedef struct {
    uint32_t clb;
	uint32_t clbu;
	uint32_t fb;
	uint32_t fbu;
	uint32_t ints;
	uint32_t ie;
	uint32_t cmd;
	uint32_t reserved0;
	uint32_t tfd;
	uint32_t sig;
	uint32_t ssts;
	uint32_t sstl;
	uint32_t serr;
	uint32_t sact;
	uint32_t ci;
	uint32_t sntf;
	uint32_t fbs;
	uint32_t devslp;
	uint32_t reserved1[11];
	uint32_t vs[10];
} ahci_port_regs_t;

typedef struct {
	uint32_t cap;
	uint32_t ghc;
	uint32_t ints;
	uint32_t pi;
	uint32_t vs;
	uint32_t ccc_ctl;
	uint32_t ccc_ports;
	uint32_t em_lock;
	uint32_t em_ctl;
	uint32_t cap2;
	uint32_t bohc;
	uint32_t reserved[29];
	uint32_t vendor[24];
} ahci_regs_t;

typedef struct {
    ahci_port_regs_t regs;
    int32_t status;
} ahci_dev_t;

typedef struct {
    pci_bar_t pci_bar;
    ahci_regs_t* regs;

    ahci_dev_t* devs;
    uint32_t devs_count;

    uint32_t port_count;
    uint32_t cmd_slots;
    uint16_t version_maj;
    uint16_t version_min;

} ahci_controller_t;

void init_ahci(void);

#endif // AHCI_H_
