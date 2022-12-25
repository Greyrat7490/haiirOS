#ifndef H_PAGE
#define H_PAGE

#include "types.h"

#define PAGE_SIZE 4096  // 4KiB

typedef struct
{
    uint64_t start_addr;
} hPage;

hPage get_hPage(uint64_t containing_addr);

#endif

