#include "ahci.h"
#include "io/io.h"
#include "pci/pci.h"
#include "memory/phys.h"
#include "memory/paging.h"
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

static uint8_t get_ahci_dev_type(ahci_port_regs_t* port) {
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

static volatile ahci_cmd_table* set_prdt(volatile ahci_cmd_header* cmd_header, uint64_t buffer_addr, bool interrupt, uint32_t byte_count) {
    volatile ahci_cmd_table* table = (ahci_cmd_table*) ((uint64_t) cmd_header->ctba | ((uint64_t) cmd_header->ctbau << 32));

    table->entries[0].i = interrupt & 1;
    table->entries[0].dbc = byte_count;
    table->entries[0].dba = (uint32_t) buffer_addr;
    table->entries[0].dbau = (uint32_t) (buffer_addr >> 32);

    return table;
}


static void start_cmd(ahci_port_regs_t* port) {
    while ((port->cmd & HBA_CMD_CR) != 0) {} 

    port->cmd |= HBA_CMD_FRE;
    port->cmd |= HBA_CMD_ST;
}

static void stop_cmd(ahci_port_regs_t* port) {
    port->cmd &= ~HBA_CMD_ST;
    port->cmd &= ~HBA_CMD_FRE;

    while ((port->cmd & HBA_CMD_CR) != 0 || (port->cmd & HBA_CMD_FR) != 0) {} 
}

static void rebase_port(ahci_port_regs_t* port) {
    stop_cmd(port);

    uint64_t cmd_list_addr = (uint64_t) pmm_alloc(1);
    for (uint64_t i = 0; i < FRAME_SIZE; i++) { ((uint8_t*)cmd_list_addr)[i] = 0; }

    port->clb = (uint32_t) cmd_list_addr;
    port->clbu = (uint32_t) (cmd_list_addr >> 32);

    for (uint32_t i = 0; i < 32; i++) {
        volatile ahci_cmd_header* cmd_header = (ahci_cmd_header*) (((uint64_t) port->clb | ((uint64_t) port->clbu << 32))
                + i * sizeof(ahci_cmd_header));

        uint64_t desc_base = (uint64_t) pmm_alloc(1);
        for (uint64_t i = 0; i < FRAME_SIZE; i++) { ((uint8_t*)desc_base)[i] = 0; }

        cmd_header->ctba = (uint32_t) desc_base;
        cmd_header->ctbau = (uint32_t) (desc_base >> 32);
        cmd_header->prdtl = 1;
    }

    uint64_t fib_addr = (uint64_t) pmm_alloc(1);
    for (uint64_t i = 0; i < FRAME_SIZE; i++) { ((uint8_t*)fib_addr)[i] = 0; }

    port->fb = (uint32_t) fib_addr;
    port->fbu = (uint32_t) (fib_addr >> 32);

    start_cmd(port);
}

static void send_cmd(ahci_port_regs_t* port, uint32_t slot) {
    // wait until port is not busy anymore
    while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) != 0) { __asm__ volatile ("pause"); }

    // issue command
    port->ci = 1 << slot;

    // wait for completion
    while ((port->ci & (1 << slot)) != 0) { __asm__ volatile ("pause"); } 
}

static bool bios_handoff(ahci_regs_t* regs) {
    if ((regs->cap2 & 1) == 0) {
        kprintln("WARNING: AHCI controller does not support bios handoff");     // not needed on all hardware?
        return false;
    }

    regs->bohc |= 1 << 1;

    while ((regs->bohc & 1) == 0) { __asm__ volatile ("pause"); }

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

static uint32_t find_cmd_slot(uint32_t cmd_slots, ahci_port_regs_t* port) {
    for (uint32_t i = 0; i < cmd_slots; i++) {
        if (((port->ci | port->sact) & (1 << i)) == 0) {
            return i;
        }
    }

    return (uint32_t)-1;
}

static bool send_identify_cmd(uint16_t* ident, uint32_t cmd_slots, ahci_port_regs_t* port) {
    uint32_t cmd_slot = find_cmd_slot(cmd_slots, port);
    if (cmd_slot == (uint32_t)-1) {
        kprintln("ERROR: no free command slot (retry later)");
        return false;
    }

    volatile ahci_cmd_header* cmd_header = (ahci_cmd_header*) (((uint64_t) port->clb | ((uint64_t) port->clbu << 32))
            + cmd_slot * sizeof(ahci_cmd_header));

    cmd_header->cfl = sizeof(ahci_fis_h2d) / 4;
    cmd_header->w = 0;
    cmd_header->prdtl = 1;

    volatile ahci_cmd_table* cmd_table = set_prdt(cmd_header, (uint64_t)ident, true, 511);

    volatile ahci_fis_h2d* cmd_ptr = (ahci_fis_h2d*) &cmd_table->cfis;
    for (uint64_t i = 0; i < sizeof(ahci_fis_h2d); i++) { ((uint8_t*)cmd_ptr)[i] = 0; } // TODO: memset

    cmd_ptr->cmd = 0xec; // identify command
    cmd_ptr->c = 1;      // command
    cmd_ptr->type = FIS_TYPE_REG_H2D;
    cmd_ptr->device = 0;

    send_cmd(port, cmd_slot);
    return true;
}

static bool send_rw_cmd(uint64_t lba, uint16_t sector_count, void* buffer, bool write, ahci_dev_t* dev) {
    uint32_t cmd_slot = find_cmd_slot(dev->cmd_slots, dev->port);
    if (cmd_slot == (uint32_t)-1) {
        kprintln("ERROR: no free command slot (retry later)");
        return false;
    }

    volatile ahci_cmd_header* cmd_header = (ahci_cmd_header*) (((uint64_t) dev->port->clb | ((uint64_t) dev->port->clbu << 32))
            + cmd_slot * sizeof(ahci_cmd_header));

    cmd_header->cfl = sizeof(ahci_fis_h2d) / 4;
    cmd_header->w = write & 1;
    cmd_header->prdtl = 1;          // TODO: use multiple entries

    volatile ahci_cmd_table* cmd_table = set_prdt(cmd_header, (uint64_t)buffer, true, 511);

    volatile ahci_fis_h2d* cmd_ptr = (ahci_fis_h2d*) &cmd_table->cfis;
    for (uint64_t i = 0; i < sizeof(ahci_fis_h2d); i++) { ((uint8_t*)cmd_ptr)[i] = 0; }

    if (write) {
        cmd_ptr->cmd = 0x35;    // write command
    } else {
        cmd_ptr->cmd = 0x25;    // read command
    }
    cmd_ptr->c = 1;             // command
    cmd_ptr->type = FIS_TYPE_REG_H2D;
    cmd_ptr->device = 1<<6;     // lba mode

    cmd_ptr->lba0 = lba & 0xff;
    cmd_ptr->lba1 = (lba >> 8) & 0xff;
    cmd_ptr->lba2 = (lba >> 16) & 0xff;
    cmd_ptr->lba3 = (lba >> 24) & 0xff;
    cmd_ptr->lba4 = (lba >> 32) & 0xff;
    cmd_ptr->lba5 = (lba >> 40) & 0xff;

    cmd_ptr->countl = sector_count & 0xff;
    cmd_ptr->counth = (sector_count >> 8) & 0xff;

    send_cmd(dev->port, cmd_slot);
    return true;
}

static ahci_dev_t init_ahci_dev(uint32_t cmd_slots, ahci_port_regs_t* port) {
    rebase_port(port);

    uint16_t* ident = (uint16_t*) pmm_alloc(1);
    if (!send_identify_cmd(ident, cmd_slots, port)) {
        kprintln("ERROR: could not send identify command");
        return (ahci_dev_t){0};
    }

    uint64_t sector_count = *((uint64_t*)(&ident[100]));
    kprintln("ahci dev sector count: %d", sector_count);

    char* info = pmm_alloc(1);
    char* serial_num = info;
    char* firmware = info + 21;
    char* model_num = info + 30;

    // serial_num
    for (uint64_t i = 0; i < 10; i++) {
        // swap bytes
        serial_num[i*2+1] = (uint8_t) (ident[10+i] & 0xff);
        serial_num[i*2] = (uint8_t) (ident[10+i] >> 8) & 0xff;
    }

    // firmware
    for (uint64_t i = 0; i < 4; i++) {
        // swap bytes
        firmware[i*2+1] = (uint8_t) (ident[23+i] & 0xff);
        firmware[i*2] = (uint8_t) (ident[23+i] >> 8) & 0xff;
    }

    // model_num
    for (uint64_t i = 0; i < 20; i++) {
        // swap bytes
        model_num[i*2+1] = (uint8_t) (ident[27+i] & 0xff);
        model_num[i*2] = (uint8_t) (ident[27+i] >> 8) & 0xff;
    }

    kprintln("ahci dev serial number: %s", serial_num);
    kprintln("ahci dev firmware: %s", firmware);
    kprintln("ahci dev model number: %s", model_num);

    // TODO: check device kind (should be AHCI_DEV_SATA)

    return (ahci_dev_t) {
        .port=port,
        .cmd_slots=cmd_slots,
        .sector_count=sector_count,
        .serial_num=serial_num,
        .firmware=firmware,
        .model_num=model_num,
    };
}

static bool init_ahci_controller(pci_dev_t* dev) {
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
        map_frame(to_page(bar.base + i*PAGE_SIZE), to_frame(bar.base + i*PAGE_SIZE), Present | Writeable | DisableCache);
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

ahci_dev_t* get_ahci_dev(uint32_t controller, uint32_t device) {
    if (controller < controllers_count) {
        if (device < controllers[controller].devs_count) {
            return &controllers[controller].devs[device];
        }
    }

    return 0x0;
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

uint64_t ahci_read(uint64_t location, uint64_t count, void *buffer, ahci_dev_t* dev) {
    // TODO: handle not AHCI_SECTOR_SIZE aligned location and sector_count
    uint64_t lba = location / AHCI_SECTOR_SIZE;
    uint64_t sector_count = count / AHCI_SECTOR_SIZE;

    uint64_t buf_size = (count + FRAME_SIZE-1) & ~(FRAME_SIZE-1);
    void* buf = pmm_alloc(buf_size);

    if (!send_rw_cmd(lba, sector_count, buf, false, dev)) {
        pmm_free(to_frame((uint64_t)buf), buf_size);
        kprintln("ERROR: could not read from AHCI device");
        return 0;
    }

    // TODO: memcpy
    for (uint64_t i = 0; i < count; i++) {
        ((uint8_t*)buffer)[i] = ((uint8_t*)buf)[i];
    }

    pmm_free(to_frame((uint64_t)buf), buf_size);

    return count;
}

uint64_t ahci_write(uint64_t location, uint64_t count, void *buffer, ahci_dev_t* dev) {
    uint64_t lba = location / AHCI_SECTOR_SIZE;
    uint64_t sector_count = count / AHCI_SECTOR_SIZE;

    uint64_t buf_size = (count + FRAME_SIZE-1) & ~(FRAME_SIZE-1);
    void* buf = pmm_alloc(buf_size);

    // TODO: memcpy
    for (uint64_t i = 0; i < count; i++) {
        ((uint8_t*)buf)[i] = ((uint8_t*)buffer)[i];
    }

    if (!send_rw_cmd(lba, sector_count, buf, true, dev)) {
        pmm_free(to_frame((uint64_t)buf), buf_size);
        kprintln("ERROR: could not write to AHCI device");
        return 0;
    }

    pmm_free(to_frame((uint64_t)buf), buf_size);

    return count;
}
