#include "hPage.h"

static uint32_t PAGE_SIZE = 4096; //4KiB

hPage get_hPage( uint64_t containing_addr ) {
    hPage page = {
        ( uint64_t* ) ( containing_addr - ( containing_addr % PAGE_SIZE ) )
    };

    return page;
}