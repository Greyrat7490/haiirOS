#include "hPage.h"

hPage get_hPage(uint64_t containing_addr) {
    hPage page = {
        containing_addr - (containing_addr % PAGE_SIZE)
    };

    return page;
}

