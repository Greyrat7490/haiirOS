#include "hFrame.h"

static uint16_t FRAME_SIZE = 4096; //4KiB
static uint64_t NEXT_FRAME = 0x800000;

hFrame get_hFrame(uint64_t containing_addr) {
    hFrame frame = {
        containing_addr - (containing_addr % FRAME_SIZE)
    };

    return frame;
}

uint64_t get_next_frame_addr() {
    return NEXT_FRAME;
}

// linear allocator
// TODO: proper allocator
hFrame alloc_frame() {
    hFrame new_frame = { NEXT_FRAME };

    NEXT_FRAME += FRAME_SIZE;

    return new_frame;
}
