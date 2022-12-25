#include "hFrame.h"

static uint64_t NEXT_FRAME = 0x800000;

hFrame get_hFrame(uint64_t containing_addr) {
    hFrame frame = {
        containing_addr - (containing_addr % FRAME_SIZE)
    };

    return frame;
}

uint64_t get_next_frame_addr(void) {
    return NEXT_FRAME;
}

// linear allocator
// TODO: proper allocator
hFrame alloc_frame(void) {
    hFrame new_frame = { NEXT_FRAME };

    NEXT_FRAME += FRAME_SIZE;

    return new_frame;
}
