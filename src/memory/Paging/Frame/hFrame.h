#ifndef H_FRAME
#define H_FRAME

#include "types.h"

typedef struct
{
    uint64_t start_addr;
} hFrame;

hFrame get_hFrame( uint64_t containing_addr );

#endif