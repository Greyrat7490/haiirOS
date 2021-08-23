#include "hFrame.h"

static uint32_t FRAME_SIZE = 4096; //4KiB

hFrame get_hFrame( uint64_t containing_addr ) {
    hFrame frame = {
        containing_addr - ( containing_addr % FRAME_SIZE )
    };

    return frame;
}