#ifndef H_PAGE
#define H_PAGE

#include "types.h"

typedef struct
{
    uint64_t start_addr;
} hPage;

hPage get_hPage( uint64_t containing_addr );

#endif