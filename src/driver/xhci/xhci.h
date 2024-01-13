#ifndef XHCI_H_
#define XHCI_H_

#include "pci/pci.h"
#include "types.h"

typedef struct {
    uint32_t route_string:20;
    uint8_t speed:4;
    uint8_t rsvd1:1;
    uint8_t mtt:1;
    uint8_t hub:1;
    uint8_t context_entry:5;
    uint16_t max_exit_latency;
    uint8_t root_hub_port_num;
    uint8_t num_ports;
    uint8_t parent_hub_slot;
    uint8_t parent_port;
    uint8_t ttt:2;
    uint8_t rsvd2:4;
    uint16_t interrupt_target:10;
    uint8_t usb_dev_addr;
    uint32_t rsvd3:19;
    uint8_t slot_state:5;
    uint64_t rsvd4;
    uint64_t rsvd5;
} __attribute__((packed)) xhci_slot_context_t;

typedef struct {
    uint8_t state:3;
    uint8_t rsvd1:5;
    uint8_t mult:2;
    uint8_t max_primary_streams:5;
    uint8_t lin_stream_arr:1;
    uint8_t interval;
    uint8_t max_esit_payload_high;
    uint8_t rsvd2:1;
    uint8_t err_count:2;
    uint8_t type:3;
    uint8_t rsvd3:1;
    uint8_t host_init_disable:1;
    uint8_t max_burst_size;
    uint16_t max_packet_size;
    uint8_t dequeue_cycle_state:1;
    uint8_t rsvd4:3;
    uint64_t tr_dequeue_ptr:60;
    uint16_t avg_trb_len;
    uint16_t max_esit_payload_low;
    uint64_t rsvd5;
    uint32_t rsvd6;
} __attribute__((packed)) xhci_endpoint_context_t;

typedef struct {
    xhci_slot_context_t slot_ctx;
    xhci_endpoint_context_t endpoint_ctxs[31];
} __attribute__((packed)) xhci_dev_context_t;

typedef struct {
    uint64_t addr;
    uint32_t status;
    uint32_t flags;
} __attribute__((packed)) xhci_trb_t;


typedef struct {
    uint64_t base_addr; // 64B bound
    uint16_t size;
    uint16_t rsvd1;
    uint32_t rsvd2;
} __attribute__((packed)) xhci_erst_entry_t;

typedef struct {
    pci_bar_t bar;
    uint8_t cap_len;
    uint8_t max_ports;
    uint8_t max_dev_slots;
    uint16_t erst_max;
    uint64_t cap_regs;
    uint64_t op_regs;
    uint64_t runtime_regs;
    uint64_t doorbell_array;
    volatile xhci_dev_context_t* dev_ctxs;
    volatile xhci_erst_entry_t* erst;
    volatile xhci_trb_t* cmd_ring;
    uint32_t cmd_ring_idx;
    uint8_t cmd_pcs;
    uint32_t event_idx;
    uint8_t event_ccs;
    bool context_size_64;
} xhci_controller_t;

void init_xhci(void);

#endif // XHCI_H_
