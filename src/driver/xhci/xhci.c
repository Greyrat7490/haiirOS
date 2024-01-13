#include "xhci.h"
#include "interrupt/asm.h"
#include "memory/paging.h"
#include "pci/pci.h"
#include "io/io.h"

#define MAX_XHCI_CONTROLLER 32
#define XHCI_MAX_EVENTS (16 * 2)
#define XHCI_MAX_COMMANDS (16 * 2)
#define XHCI_MAX_SLOTS 255
#define XHCI_MAX_PORTS 127
#define XHCI_MAX_ENDPOINTS 32
#define XHCI_MAX_SCRATCHPADS 256    // only 256 instead of 1023 to stay in page boundary
#define XHCI_MAX_DEVICES 128
#define XHCI_MAX_TRANSFERS 8
#define XHCI_MAX_INTERRUPTER 1024
#define XHCI_DOORBELL_ARR_LEN 256

// host controller capabilities
#define HCC_CSZ (1 << 2)
#define HCC_XECP(p) ((p >> 16) & 0xffff)
#define HCS_MAX_PORTS(p) ((p >> 24) & 0xff)
#define HCS_MAX_DEV_SLOTS(p) (p & 0xff)
#define HCS2_ERST_MAX(p) (2 << ((p >> 4) & 0xf))
#define HCS2_MAX_SCRATCHPAD_BUFS(p) (((p >> 21) & 0x1f << 5) | ((p >> 27) & 0x1f))

#define XECP_ID(p) (*p & 0xff)
#define XECP_NEXT(p) ((*p >> 8) & 0xff)
#define XECP_LEGSUP_ID 0x1
#define XECP_LEGSUP_BIOS_OWNED (1 << 16)
#define XECP_LEGSUP_OS_OWNED (1 << 24)
#define XECP_LEGCTLSTS 0x4
#define XECP_LEGCTLSTS_DISABLE_SMI ((0x7 << 1) + (0xff << 5) + (0x7 << 17))
#define XECP_LEGCTLSTS_EVENTS_SMI (0x7 << 29)

// runtime registers
#define RUNTIME_REG_IMAN(interrupter) (0x20 + (interrupter * 32))
#define RUNTIME_REG_IMOD(interrupter) (0x24 + (interrupter * 32))
#define RUNTIME_REG_ERSTSZ(interrupter) (0x28 + (interrupter * 32))
#define RUNTIME_REG_ERSTBA(interrupter) (0x30 + (interrupter * 32))
#define RUNTIME_REG_ERDP(interrupter) (0x38 + (interrupter * 32))

// operation registers
#define OP_REG_USBCMD 0x0
#define OP_REG_USBSTS 0x4
#define OP_REG_PAGESIZE 0x8
#define OP_REG_DNCTRL 0x14
#define OP_REG_CRCR 0x18
#define OP_REG_DCBAAP 0x30
#define OP_REG_CONFIG 0x38
#define OP_REG_PORT_SET(p) (0x400+(p*0x10))

#define CRCR_HCS (1 << 0)
#define CRCR_CS (1 << 1)
#define CRCR_CA (1 << 2)
#define CRCR_RUNNING (1 << 3)
#define CRCR_PTR_MASK (~63ull)

#define USBCMD_RUN (1 << 0)
#define USBCMD_HCRST (1 << 1)
#define USBCMD_INTE (1 << 2)
#define USBCMD_HSEE (1 << 3)
#define USBCMD_LHCRST (1 << 7)
#define USBCMD_CSS (1 << 8)
#define USBCMD_CRS (1 << 9)
#define USBCMD_EWE (1 << 10)

#define USBSTS_HCH (1 << 0)
#define USBSTS_HSE (1 << 2)
#define USBSTS_EINT (1 << 3)
#define USBSTS_PCD (1 << 4)
#define USBSTS_SSS (1 << 8)
#define USBSTS_RSS (1 << 9)
#define USBSTS_SRE (1 << 10)
#define USBSTS_CNR (1 << 11)
#define USBSTS_HCE (1 << 12)

#define PORTSC 0
#define PORTPMSC 0x4
#define PORTLI 0x8
#define PORTHLPMC 0xc

#define TRB_CYCLE_BIT (1U << 0)
#define TRB_TC_BIT (1U << 1)
#define TRB_BSR_BIT (1U << 9)

#define TRB_TYPE 10
#define TRB_SLOT_TYPE 16
#define TRB_SLOT_ID 24

// TRB types for transfer ring 
#define TRB_TYPE_NORMAL         (1 << TRB_TYPE)
#define TRB_TYPE_SETUP_STAGE    (2 << TRB_TYPE)
#define TRB_TYPE_DATA_STAGE     (3 << TRB_TYPE)
#define TRB_TYPE_STATUS_STAGE   (4 << TRB_TYPE)
#define TRB_TYPE_ISOCH          (5 << TRB_TYPE)
#define TRB_TYPE_LINK           (6 << TRB_TYPE) // and for command ring
#define TRB_TYPE_EVENT_DATA     (7 << TRB_TYPE)
#define TRB_TYPE_NO_OP          (8 << TRB_TYPE)

// TRB types for command ring 
#define TRB_TYPE_ENABLE_SLOT_CMD        (9 << TRB_TYPE)
#define TRB_TYPE_DISABLE_SLOT_CMD       (10 << TRB_TYPE)
#define TRB_TYPE_ADDR_DEV_CMD           (11 << TRB_TYPE)
#define TRB_TYPE_CONF_ENDPOINT_CMD      (12 << TRB_TYPE)
#define TRB_TYPE_EVAL_CTX_CMD           (13 << TRB_TYPE)
#define TRB_TYPE_RESET_ENDPOINT_CMD     (14 << TRB_TYPE)
#define TRB_TYPE_STOP_ENDPOINT_CMD      (15 << TRB_TYPE)
#define TRB_TYPE_SET_TR_DEQ_PTR_CMD     (16 << TRB_TYPE)
#define TRB_TYPE_RESET_DEV_CMD          (17 << TRB_TYPE)
#define TRB_TYPE_FORCE_EVENT_CMD        (18 << TRB_TYPE)
#define TRB_TYPE_NEGOT_BANDWIDTH_CMD    (19 << TRB_TYPE)
#define TRB_TYPE_SET_LATENCY_TOL_CMD    (20 << TRB_TYPE)
#define TRB_TYPE_GET_PORT_BANDWIDTH_CMD (21 << TRB_TYPE)
#define TRB_TYPE_FORCE_HEADER_CMD       (22 << TRB_TYPE)
#define TRB_TYPE_NO_OP_CMD              (23 << TRB_TYPE)
#define TRB_TYPE_GET_EXT_PROP_CMD       (24 << TRB_TYPE)
#define TRB_TYPE_SET_EXT_PROP_CMD       (25 << TRB_TYPE)

// TRB types for event ring
#define TRB_TYPE_TRANSFER_EVENT             (32 << TRB_TYPE)
#define TRB_TYPE_CMD_COMPL_EVENT            (33 << TRB_TYPE)
#define TRB_TYPE_PORT_STATUS_CHANGE_EVENT   (34 << TRB_TYPE)
#define TRB_TYPE_BANDWIDTH_REQ_EVENT        (35 << TRB_TYPE)
#define TRB_TYPE_DOORBELL_EVENT             (36 << TRB_TYPE)
#define TRB_TYPE_HOST_CONTROLLER_EVENT      (37 << TRB_TYPE)
#define TRB_TYPE_DEV_NOTIFICATION_EVENT     (38 << TRB_TYPE)
#define TRB_TYPE_MFINDEX_WRAP_EVENT         (39 << TRB_TYPE)


static xhci_controller_t controllers[MAX_XHCI_CONTROLLER];
static uint32_t controllers_count = 0;

static uint32_t read_op_reg(xhci_controller_t* controller, uint64_t reg_off) {
    return *(volatile uint32_t*)(controller->op_regs+reg_off);
}

static void write_op_reg(xhci_controller_t* controller, uint64_t reg_off, uint32_t value) {
    *(volatile uint32_t*)(controller->op_regs+reg_off) = value;
}

static void write_doorbell(xhci_controller_t* controller, uint64_t reg_off, uint16_t task_id, uint8_t target) {
    *(volatile uint32_t*)(controller->doorbell_array+reg_off) = ((uint32_t)task_id << 16) | target;
}

static void ring(xhci_controller_t* controller, uint16_t task_id, uint8_t target) {
    write_doorbell(controller, 0, task_id, target);
}

static uint32_t read_runtime(xhci_controller_t* controller, uint32_t reg_off) {
    return *(volatile uint32_t*)(controller->runtime_regs+reg_off);
}

static void write_runtime(xhci_controller_t* controller, uint32_t reg_off, uint32_t value) {
    *(volatile uint32_t*)(controller->runtime_regs+reg_off) = value;
}

static int32_t wait_op_bits(xhci_controller_t* controller, uint64_t reg_off, uint32_t bitmask, uint32_t expected) {
    for (uint16_t i = 0; i < 1000; i++) {
        if ((read_op_reg(controller, reg_off) & bitmask) == expected) {
            return 0; 
        }

        for (uint32_t j = 0; j < 100000; j++) { __asm__ volatile ("pause"); }
    }

    return -1;
}

static bool halt_controller(xhci_controller_t* controller) {
    write_op_reg(controller, OP_REG_USBCMD, read_op_reg(controller, OP_REG_USBCMD) & ~USBCMD_RUN);
    if (wait_op_bits(controller, OP_REG_USBSTS, USBSTS_HCH, USBSTS_HCH)) {
        kprintln("ERROR: could not halt XHCI controller");
        return false;
    }

    return true;
}

static bool reset_controller(xhci_controller_t* controller) {
    write_op_reg(controller, OP_REG_USBCMD, read_op_reg(controller, OP_REG_USBCMD) | USBCMD_HCRST);
    if (wait_op_bits(controller, OP_REG_USBCMD, USBCMD_HCRST, 0)) {
        kprintln("ERROR: could not reset XHCI controller (USBCMD_HCRST != 0)");
        return false;
    }
    if (wait_op_bits(controller, OP_REG_USBSTS, USBSTS_CNR, 0)) {
        kprintln("ERROR: could not reset XHCI controller (USBSTS_CNR != 0)");
        return false;
    }

    return true;
}

static void bios_handoff(uint32_t base, uint32_t cap_params) {
    volatile uint32_t* xecp = (uint32_t*)((uint64_t)base + (HCC_XECP(cap_params) << 2));

    uint8_t id = XECP_ID(xecp);
    while (id != XECP_LEGSUP_ID) {
        if (XECP_NEXT(xecp) == 0) { return; }

        xecp += XECP_NEXT(xecp);
        id = XECP_ID(xecp);
    }

    if (*xecp & XECP_LEGSUP_BIOS_OWNED) {
        *xecp |= XECP_LEGSUP_OS_OWNED;

        for (uint8_t i = 0; i < 20; i++) {
            if ((*xecp & XECP_LEGSUP_BIOS_OWNED) == 0) { break; }

            for (uint32_t j = 0; j < 100 * 1000; j++) { io_wait(); } // wait ~0.1s
        }

        // clear BIOS owned flag because some BIOSs might not which could cause problems
        *xecp &= ~XECP_LEGSUP_BIOS_OWNED;

        uint32_t legctlsts = *((volatile uint32_t*)((uint64_t)xecp + XECP_LEGCTLSTS));
        legctlsts &= ~XECP_LEGCTLSTS_DISABLE_SMI;
        legctlsts |= XECP_LEGCTLSTS_EVENTS_SMI;
        *((volatile uint32_t*)((uint64_t)xecp + XECP_LEGCTLSTS)) = legctlsts;

        kprintln("handed off xhci controller to OS");
    }
}

static void init_dcbaa(xhci_controller_t* controller) {
    uint64_t size = XHCI_MAX_SLOTS * sizeof(xhci_dev_context_t);
    uint64_t dcbaa_addr = (uint64_t)pmm_alloc((size-1) / PAGE_SIZE + 1);
    controller->dev_ctxs = (xhci_dev_context_t*)dcbaa_addr;

    for (uint32_t i = 0; i < XHCI_MAX_SLOTS; i++) {
        controller->dev_ctxs[i] = (xhci_dev_context_t){0};
    }

    write_op_reg(controller, OP_REG_DCBAAP, (uint32_t)dcbaa_addr);
    write_op_reg(controller, OP_REG_DCBAAP+4, (uint32_t)(dcbaa_addr >> 32));

    kprintln("dcbaa_addr: %x (size: %d)", dcbaa_addr, size);
}

static void clear_ring(volatile xhci_trb_t* ring, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        ring[i] = (xhci_trb_t) { .addr = 0, .status = 0, .flags = 0 };
    }
}

static void init_cmd_ring(xhci_controller_t* controller) {
    uint64_t size = XHCI_MAX_COMMANDS * sizeof(xhci_trb_t);
    uint64_t cmd_ring_addr = (uint64_t)pmm_alloc_custom((size-1) / PAGE_SIZE + 1, Present | Writeable | DisableCache);
    controller->cmd_ring = (xhci_trb_t*)cmd_ring_addr;

    clear_ring(controller->cmd_ring, XHCI_MAX_COMMANDS);

    controller->cmd_ring[XHCI_MAX_COMMANDS-1].addr = cmd_ring_addr;
    controller->cmd_ring[XHCI_MAX_COMMANDS-1].status = 0;
    controller->cmd_ring[XHCI_MAX_COMMANDS-1].flags = TRB_TYPE_LINK;

    write_op_reg(controller, OP_REG_CRCR, (uint32_t)cmd_ring_addr | CRCR_HCS);
    write_op_reg(controller, OP_REG_CRCR+4, (uint32_t)(cmd_ring_addr >> 32));

    kprintln("cmd_ring_addr: %x (size %d)", cmd_ring_addr, size);
}

static xhci_erst_entry_t create_event_ring_seg(void) {
    uint32_t size = XHCI_MAX_EVENTS * sizeof(xhci_trb_t);
    volatile xhci_trb_t* event_ring = (volatile xhci_trb_t*)pmm_alloc_custom((size-1) / PAGE_SIZE + 1, Present | Writeable | DisableCache);

    clear_ring(event_ring, XHCI_MAX_EVENTS);

    kprintln("created event_ring_seg: %x (size %d)", (uint64_t)event_ring, size);

    xhci_erst_entry_t entry = {
        .size = XHCI_MAX_EVENTS,
        .base_addr = (uint64_t)event_ring
    };
    return entry;
}

static void init_event_ring_seg_table(xhci_controller_t* controller) {
    controller->erst_max = 1;

    uint32_t size = controller->erst_max * sizeof(xhci_erst_entry_t);
    uint64_t addr = (uint64_t)pmm_alloc_custom((size-1) / PAGE_SIZE + 1, Present | Writeable | DisableCache);

    controller->erst = (xhci_erst_entry_t*)addr;

    for (uint32_t i = 0; i < controller->erst_max; i++) {
        controller->erst[i] = create_event_ring_seg();
    }

    uint64_t erdp = controller->erst[0].base_addr;

    write_runtime(controller, RUNTIME_REG_ERSTSZ(0), controller->erst_max);
    write_runtime(controller, RUNTIME_REG_ERDP(0), (uint32_t)erdp);
    write_runtime(controller, RUNTIME_REG_ERDP(0)+4, (uint32_t)(erdp >> 32));
    write_runtime(controller, RUNTIME_REG_ERSTBA(0), (uint32_t)addr);
    write_runtime(controller, RUNTIME_REG_ERSTBA(0)+4, (uint32_t)(addr >> 32));

    kprintln("erst addr: %x (size %d)", addr, size);
}

static void enable_slots(xhci_controller_t* controller) {
    uint32_t config = read_op_reg(controller, OP_REG_CONFIG) & ~(uint32_t)0xff;
    config |= controller->max_dev_slots;
    write_op_reg(controller, OP_REG_CONFIG, config);
    kprintln("enabled slots: %d", read_op_reg(controller, OP_REG_CONFIG) & 0xff);
}

static void start_controller(xhci_controller_t* controller) {
    write_op_reg(controller, OP_REG_USBCMD, USBCMD_RUN);
    if (wait_op_bits(controller, OP_REG_USBSTS, USBSTS_HCH, 0)) {
        kprintln("ERROR: could not start xhci controller");
    }

    kprintln("started xhci controller");
}

static void queue_cmd(xhci_controller_t* controller, xhci_trb_t trb) {
    if (controller->cmd_pcs) {
        trb.flags |= TRB_CYCLE_BIT;
    } else {
        trb.flags &= ~TRB_CYCLE_BIT;
    }

    controller->cmd_ring[controller->cmd_ring_idx] = trb;
    controller->cmd_ring_idx++;

    if (controller->cmd_ring_idx >= XHCI_MAX_COMMANDS) {
        controller->cmd_ring[XHCI_MAX_COMMANDS-1].flags |= TRB_TC_BIT;
        if (controller->cmd_pcs) {
            controller->cmd_ring[XHCI_MAX_COMMANDS-1].flags |= TRB_CYCLE_BIT;
        }

        controller->cmd_ring_idx = 0; 
        controller->cmd_pcs ^= 1;
    }
}

static void process_event(xhci_controller_t* controller) {
    volatile xhci_trb_t* erdp = (volatile xhci_trb_t*)controller->erst[0].base_addr;

    while (1) {
        bool cycle_bit = (erdp[controller->event_idx].flags & TRB_CYCLE_BIT) != 0;
        if (cycle_bit != controller->event_ccs) {
            continue;
        }

        xhci_trb_t trb = erdp[controller->event_idx];
        kprintln("%d %x %x %x", controller->event_idx, trb.flags, trb.status, trb.addr);

        controller->event_idx++;
        if (controller->event_idx == XHCI_MAX_EVENTS) {
            controller->event_idx = 0;
            controller->event_ccs ^= 1;
        }

        write_runtime(controller, RUNTIME_REG_ERDP(0), (uint64_t)&erdp[controller->event_idx]);
        write_runtime(controller, RUNTIME_REG_ERDP(0)+4, (uint32_t)((uint64_t)&erdp[controller->event_idx] >> 32));
    }
}

static void no_op_cmd(xhci_controller_t* controller) {
    xhci_trb_t trb = { .flags = 0, .addr = 0, .status = 0 };
    trb.flags |= TRB_TYPE_NO_OP_CMD;
    queue_cmd(controller, trb);
    ring(controller, 0, 0);
}

static void enable_slot_cmd(xhci_controller_t* controller) {
    xhci_trb_t trb = { .flags = 0, .addr = 0, .status = 0 };
    trb.flags |= TRB_TYPE_ENABLE_SLOT_CMD;
    queue_cmd(controller, trb);
    ring(controller, 0, 0);
}

static bool init_xhci_controller(pci_dev_t* dev) {
    uint32_t cmd = pci_readd(dev->bus, dev->slot, dev->func, PCI_COMMAND_OFFSET);
    cmd |= PCI_CMD_MASTER | PCI_CMD_MEMORY;
    cmd &= ~(PCI_CMD_IO | PCI_CMD_INT_DISABLE);
    pci_writed(dev->bus, dev->slot, dev->func, PCI_COMMAND_OFFSET, cmd);

    if (!is_bar_present(dev, 0)) {
        kprintln("ERROR: cannot locate XHCI BAR0");
        return false;
    }

    pci_bar_t bar = get_bar(dev, 0);
    kprintln("%x %d is_64bits: %b", bar.base, bar.size, bar.is_64bits);

    if (bar.size == 0) {
        kprintln("ERROR: XHCI controller BAR0 size was 0");
        return false;
    }

    for (uint32_t i = 0; i < (bar.size-1) / PAGE_SIZE + 1; i++) {
        map_frame(to_page(bar.base + i*PAGE_SIZE), to_frame(bar.base + i*PAGE_SIZE), Present | Writeable | DisableCache);
    }

    uint32_t hcsparams1 = *(uint32_t*)(bar.base+0x4);
    uint32_t hcsparams2 = *(uint32_t*)(bar.base+0x8);
    uint32_t hccparams1 = *((uint32_t*)(bar.base+0x10));

    uint8_t max_ports = HCS_MAX_PORTS(hcsparams1);
    uint8_t max_dev_slots = HCS_MAX_DEV_SLOTS(hcsparams1);
    uint8_t erst_max = HCS2_ERST_MAX(hcsparams2);
    uint8_t max_scratchpad_bufs = HCS2_MAX_SCRATCHPAD_BUFS(hcsparams2);

    uint8_t cap_len = *((uint8_t*)bar.base);
    uint32_t rtsoff = *((uint32_t*)(bar.base+0x18));
    uint32_t dboff = *((uint32_t*)(bar.base+0x14));
    uint16_t version = *((uint16_t*)(bar.base+0x2)); 

    bool context_size_64 = (hccparams1 & HCC_CSZ) != 0;

    kprintln("cap_len: %d", cap_len);
    kprintln("max_ports: %d", max_ports);
    kprintln("max_dev_slots: %d", max_dev_slots);
    kprintln("erst_max: %d", erst_max);
    kprintln("max_scratchpad_bufs: %d", max_scratchpad_bufs);
    kprintln("rtsoff: %d", rtsoff);
    kprintln("version: %d", version);
    kprintln("64byte context: %b", context_size_64);

    uint64_t op_regs = bar.base + cap_len;
    uint64_t runtime_regs = bar.base+rtsoff;
    uint64_t doorbell_array = bar.base+dboff;

    xhci_controller_t controller = {
        .bar = bar,
        .cap_len = cap_len,
        .max_ports = max_ports,
        .max_dev_slots = max_dev_slots,
        .cap_regs = bar.base,
        .op_regs = op_regs,
        .runtime_regs = runtime_regs,
        .doorbell_array = doorbell_array,
        .context_size_64 = context_size_64,
        .cmd_pcs = 1,
        .event_ccs = 1,
        .erst_max = erst_max
    };

    controllers[controllers_count] = controller;
    controllers_count++;

    bios_handoff(bar.base, hccparams1);

    halt_controller(&controller);

    reset_controller(&controller);

    if ((read_op_reg(&controller, OP_REG_PAGESIZE) & 1) == 0) {
        kprintln("ERROR: XHCI controller does not support 4K pages");
        return false;
    }

    write_op_reg(&controller, OP_REG_USBSTS, read_op_reg(&controller, OP_REG_USBSTS));
	write_op_reg(&controller, OP_REG_DNCTRL, 0);

    enable_slots(&controller);

    init_dcbaa(&controller);
    init_cmd_ring(&controller);
    init_event_ring_seg_table(&controller);

    // disable interrupts for now
    write_runtime(&controller, RUNTIME_REG_IMOD(0), 0);
    write_runtime(&controller, RUNTIME_REG_IMAN(0), 0);
    write_op_reg(&controller, OP_REG_USBCMD, read_op_reg(&controller, OP_REG_USBCMD) & ~(uint32_t)USBCMD_INTE);

    start_controller(&controller);

    no_op_cmd(&controller);
    enable_slot_cmd(&controller);

    while (true) {
        process_event(&controller);
    }

    return true;
}

void init_xhci(void) {
    pci_dev_t* pci_devs = get_pci_devs();
    uint32_t pci_devs_len = get_pci_devs_len();

    for (uint32_t i = 0; i < pci_devs_len; i++) {
        if (pci_devs[i].class == SERIAL_BUS_CLASS) {
            if (pci_devs[i].subclass == USB_SUBCLASS && pci_devs[i].prog_if == XHCI_PROG_IF) {
                kprintln("found XHCI controller ([%x:%x:%x])", pci_devs[i].bus, pci_devs[i].slot, pci_devs[i].func);
                init_xhci_controller(&pci_devs[i]);
            }
        }
    }
}
