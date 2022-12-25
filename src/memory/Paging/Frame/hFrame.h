#ifndef H_FRAME
#define H_FRAME

#include "types.h"

#define FRAME_SIZE 4096     // 4KiB

typedef struct
{
    uint64_t start_addr;
} hFrame;

hFrame get_hFrame(uint64_t containing_addr);
hFrame alloc_frame(void);
uint64_t get_next_frame_addr(void);

#endif
