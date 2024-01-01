#include "xhci.h"
#include "interrupt/asm.h"
#include "memory/paging.h"
#include "pci/pci.h"
#include "io/io.h"

#define MAX_XHCI_CONTROLLER 128

#define HCC_CSZ (1 << 2)
#define HCC_CSZ (1 << 2)
#define HCC_XECP(p) ((p >> 16) & 0xffff)

#define XECP_ID(p) (*p & 0xff)
#define XECP_NEXT(p) ((*p >> 8) & 0xff)
#define XECP_LEGSUP_ID 0x1
#define XECP_LEGSUP_BIOS_OWNED (1 << 16)
#define XECP_LEGSUP_OS_OWNED (1 << 24)
#define XECP_LEGCTLSTS 0x4
#define XECP_LEGCTLSTS_DISABLE_SMI ((0x7 << 1) + (0xff << 5) + (0x7 << 17))
#define XECP_LEGCTLSTS_EVENTS_SMI (0x7 << 29)

#define OP_REG_USBCMD 0x0
#define OP_REG_USBSTS 0x4
#define OP_REG_PAGESIZE 0x8
#define OP_REG_DNCTRL 0x14
#define OP_REG_CRCR 0x18
#define OP_REG_DCBAAP 0x30
#define OP_REG_CONFIG 0x38

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

static xhci_controller_t controllers[MAX_XHCI_CONTROLLER];
static uint32_t controllers_count = 0;

static uint32_t read_op_reg(xhci_controller_t* controller, uint64_t reg_off) {
    return *(volatile uint32_t*)(controller->op_regs+reg_off);
}

static void write_op_reg(xhci_controller_t* controller, uint64_t reg_off, uint32_t value) {
    *(volatile uint32_t*)(controller->op_regs+reg_off) = value;
}

static bool wait_op_bits(xhci_controller_t* controller, uint64_t reg_off, uint32_t bitmask, uint32_t expected) {
    for (uint16_t i = 0; i < 1000; i++) {
        if ((read_op_reg(controller, reg_off) & bitmask) == expected) {
            return false; 
        }

        for (uint16_t j = 0; j < 1000; j++) { io_wait(); } // wait ~1ms
    }

    return true;
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

    uint8_t cap_len = *((uint8_t*)bar.base);
    uint8_t max_ports = *((uint32_t*)(bar.base+0x4)) >> 23;
    uint32_t rtsoff = *((uint32_t*)(bar.base+0x18));
    uint32_t dboff = *((uint32_t*)(bar.base+0x14));
    uint16_t version = *((uint16_t*)(bar.base+0x2)); 

    uint32_t cap_params = *((uint32_t*)(bar.base+0x10));
    bool context_size_64 = (cap_params & HCC_CSZ) != 0;

    kprintln("cap_len: %d", cap_len);
    kprintln("max_ports: %d", max_ports);
    kprintln("RTSOFF: %d", rtsoff);
    kprintln("DBOFF: %d", dboff);
    kprintln("version: %d", version);
    kprintln("64byte context: %b", context_size_64);

    uint64_t op_regs = bar.base + cap_len;
    uint64_t port_regs = bar.base + 0x400;
    uint64_t runtime_regs = bar.base + rtsoff;

    xhci_controller_t controller = {
        .bar = bar,
        .cap_len = cap_len,
        .cap_regs = bar.base,
        .op_regs = op_regs,
        .port_regs = port_regs,
        .runtime_regs = runtime_regs,
        .context_size_64 = context_size_64
    };

    controllers[controllers_count] = controller;
    controllers_count++;

    bios_handoff(bar.base, cap_params);

    kprintln("vec count: %d", pci_msi_vec_count(dev));

    halt_controller(&controller);

    reset_controller(&controller);

    if ((read_op_reg(&controller, OP_REG_PAGESIZE) & 1) == 0) {
        kprintln("ERROR: XHCI controller does not support 4K pages");
        return false;
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
