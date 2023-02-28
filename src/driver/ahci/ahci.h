#ifndef AHCI_H_
#define AHCI_H_

#include "types.h"
#include "pci/pci.h"

#define MAX_AHCI_DEVS 32

#define FIS_TYPE_REG_H2D    0x27    // Register FIS         (host to device)
#define FIS_TYPE_REG_D2H    0x34    // Register FIS         (device to host)
#define FIS_TYPE_DMA_ACT    0x39    // DMA activate FIS     (device to host)
#define FIS_TYPE_DMA_SETUP  0x41    // DMA setup FIS        (bidirectional)
#define FIS_TYPE_DATA       0x46    // Data FIS             (bidirectional)
#define FIS_TYPE_BIST       0x58    // BIST activate FIS    (bidirectional)

#define HBA_CMD_ST    0x0001
#define HBA_CMD_FRE   0x0010
#define HBA_CMD_FR    0x4000
#define HBA_CMD_CR    0x8000                                   

typedef volatile struct {
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
    uint32_t reserved1[11];
    uint32_t vs[4];
} __attribute__((packed)) ahci_port_regs_t;

typedef volatile struct {
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
} __attribute__((packed)) ahci_regs_t;

typedef struct {
    ahci_port_regs_t* regs;
    int32_t status;
    uint64_t sector_count;
    char* serial_num;
    char* firmware;
    char* model_num;
} ahci_dev_t;

typedef struct {
    pci_bar_t pci_bar;
    ahci_regs_t* regs;

    ahci_dev_t devs[MAX_AHCI_DEVS];
    uint32_t devs_count;

    uint32_t ports_count;
    uint32_t cmd_slots;
    uint16_t version_maj;
    uint16_t version_min;

} ahci_controller_t;

typedef struct {
    uint8_t  cfl:5;             // cmd FIS length in dwords (2 ~ 16)
    uint8_t  a:1;               // atapi
    uint8_t  w:1;               // write: D2H = 0 / H2D = 1
    uint8_t  p:1;               // prefetchable
    uint8_t  r:1;               // reset
    uint8_t  b:1;               // bIST
    uint8_t  c:1;               // clear busy upon R_OK
    uint8_t  reserved0:1;
    uint8_t  pmp:4;             // port multiplier port

    uint16_t prdtl;             // physical region descriptor table length (in entries)

    volatile uint32_t prdbc;    // physical region descriptor byte count transferred

    uint32_t ctba;              // command table descriptor base address
    uint32_t ctbau;             // command table descriptor base address (upper)
    uint32_t reserved1[4];
} __attribute__((packed)) ahci_cmd_header;

typedef struct {
    uint32_t dba;           // data base address
    uint32_t dbau;          // data base address (upper)
    uint32_t reserved0;
    uint32_t dbc:22;        // byte count 4MiB max
    uint32_t reserved1:9;
    uint32_t i:1;           // interrupt on completion
} __attribute__((packed)) ahci_prdt_entry;

typedef struct {
    uint8_t cfis[64];           // cmd fis
    uint8_t acmd[16];           // atapi cmd (12 or 16 bytes)
    uint8_t reserved[48];
    ahci_prdt_entry entries[1]; // physical region descriptor table entries (0 - 65535)
} __attribute__((packed)) ahci_cmd_table;

typedef struct {
    uint8_t  type;          // = FIS_TYPE_REG_H2D

    uint8_t  pmport:4;      // port multiplier
    uint8_t  reserved0:3;
    uint8_t  c:1;           // 0: control / 1: command

    uint8_t  cmd;           // command reg
    uint8_t  featurel;      // feature reg lower

    uint8_t  lba0;          // LBA reg (7:0)
    uint8_t  lba1;          // LBA reg (15:8)
    uint8_t  lba2;          // LBA reg (23:16)
    uint8_t  device;        // device reg

    uint8_t  lba3;          // LBA reg (31:24)
    uint8_t  lba4;          // LBA reg (39:32)
    uint8_t  lba5;          // LBA reg (47:40)
    uint8_t  featureh;      // feature reg upper

    uint8_t  countl;        // count reg lower
    uint8_t  counth;        // count reg upper
    uint8_t  icc;           // isochronous command completion
    uint8_t  control;       // control reg

    uint8_t  reserved1[4];
} __attribute__((packed)) ahci_fis_h2d;

typedef struct {
    uint8_t  type;          // = FIS_TYPE_REG_D2H

    uint8_t  pmport:4;      // port multiplier
    uint8_t  reserved0:2;
    uint8_t  i:1;           // interrupt bit
    uint8_t  reserved1:1;

    uint8_t  status;
    uint8_t  error;

    uint8_t  lba0;          // LBA reg (7:0)
    uint8_t  lba1;          // LBA reg (15:8)
    uint8_t  lba2;          // LBA reg (23:16)
    uint8_t  device;        // device reg

    uint8_t  lba3;          // LBA reg (31:24)
    uint8_t  lba4;          // LBA reg (39:32)
    uint8_t  lba5;          // LBA reg (47:40)
    uint8_t  reserved2;

    uint8_t  countl;        // count reg lower
    uint8_t  counth;        // count reg upper
    uint8_t  reserved3[2];

    uint8_t  reserved4[4];
} __attribute__((packed)) ahci_fis_d2h;

void init_ahci(void);

#endif // AHCI_H_
